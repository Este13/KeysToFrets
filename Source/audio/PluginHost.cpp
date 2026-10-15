#include "audio/PluginHost.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace keystofrets
{
    namespace
    {
        constexpr DWORD cppExceptionCode = 0xE06D7363;

        bool runGuarded (void (*function) (void*), void* context)
        {
            __try
            {
                function (context);
                return true;
            }
            __except (GetExceptionCode() == cppExceptionCode ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER)
            {
                return false;
            }
        }

        template <typename Callable>
        bool guarded (Callable&& callable)
        {
            try
            {
                return runGuarded ([] (void* p) { (*static_cast<Callable*> (p)) (); }, &callable);
            }
            catch (...)
            {
                return false;
            }
        }

        juce::File bundleRootFor (const juce::File& chosen)
        {
            for (auto candidate = chosen; candidate.getFullPathName().isNotEmpty(); candidate = candidate.getParentDirectory())
            {
                if (candidate.hasFileExtension (".vst3") && candidate.isDirectory())
                    return candidate;
                if (candidate == candidate.getParentDirectory())
                    break;
            }
            return chosen;
        }
    }

    PluginHost::PluginHost() = default;

    juce::File PluginHost::normalisePath (const juce::File& chosen)
    {
        return bundleRootFor (chosen);
    }

    PluginLoadResult PluginHost::load (const juce::File& file, double sampleRate, int bufferSize)
    {
        PluginLoadResult result;
        if (! file.exists())
        {
            result.error = "The plugin file no longer exists.";
            return result;
        }

        juce::OwnedArray<juce::PluginDescription> descriptions;
        const bool scanned = guarded ([&] { format.findAllTypesForFile (descriptions, file.getFullPathName()); });
        if (! scanned)
        {
            result.error = "The plugin crashed while it was being scanned and was not loaded.";
            return result;
        }
        if (descriptions.isEmpty())
        {
            result.error = "No VST3 plugin was found in " + file.getFileName() + ".";
            return result;
        }

        const juce::PluginDescription* chosen = descriptions[0];
        for (const auto* description : descriptions)
            if (description->isInstrument)
            {
                chosen = description;
                break;
            }

        juce::String creationError;
        const bool created = guarded ([&]
        {
            result.instance = format.createInstanceFromDescription (*chosen, sampleRate, bufferSize, creationError);
        });
        if (! created)
        {
            result.instance.reset();
            result.error = "The plugin crashed while it was being created and was not loaded.";
            return result;
        }
        if (result.instance == nullptr)
        {
            result.error = creationError.isNotEmpty() ? creationError : "The plugin could not be created.";
            return result;
        }

        result.name = chosen->name;
        return result;
    }

    juce::String PluginHost::saveState (juce::AudioPluginInstance& instance)
    {
        juce::MemoryBlock block;
        const bool ok = guarded ([&] { instance.getStateInformation (block); });
        return ok && block.getSize() > 0 ? block.toBase64Encoding() : juce::String();
    }

    bool PluginHost::restoreState (juce::AudioPluginInstance& instance, const juce::String& base64)
    {
        juce::MemoryBlock block;
        if (base64.isEmpty() || ! block.fromBase64Encoding (base64))
            return false;
        return guarded ([&] { instance.setStateInformation (block.getData(), static_cast<int> (block.getSize())); });
    }
}
