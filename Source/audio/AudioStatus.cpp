#include "audio/AudioStatus.h"

namespace keystofrets
{
    juce::String driverLabelFor (const juce::String& typeName)
    {
        if (typeName == "Windows Audio (Exclusive Mode)")
            return "WASAPI Exclusive";
        if (typeName == "Windows Audio (Low Latency Mode)")
            return "WASAPI Shared (low latency)";
        if (typeName == "Windows Audio")
            return "WASAPI Shared";
        if (typeName == "ASIO")
            return "ASIO";
        return typeName;
    }

    bool isSupportedDriverType (const juce::String& typeName)
    {
        return typeName.startsWith ("Windows Audio") || typeName == "ASIO";
    }

    juce::var AudioStatus::toVar() const
    {
        juce::Array<juce::var> typeList;
        for (const auto& t : types)
        {
            auto* item = new juce::DynamicObject();
            item->setProperty ("id", t.id);
            item->setProperty ("label", t.label);
            typeList.add (juce::var (item));
        }

        juce::Array<juce::var> deviceList;
        for (const auto& d : devices)
            deviceList.add (d);

        juce::Array<juce::var> rateList;
        for (const auto r : sampleRates)
            rateList.add (r);

        juce::Array<juce::var> bufferList;
        for (const auto b : bufferSizes)
            bufferList.add (b);

        auto* root = new juce::DynamicObject();
        root->setProperty ("types", typeList);
        root->setProperty ("type", type);
        root->setProperty ("driverLabel", driverLabel);
        root->setProperty ("devices", deviceList);
        root->setProperty ("device", device);
        root->setProperty ("sampleRates", rateList);
        root->setProperty ("sampleRate", sampleRate);
        root->setProperty ("bufferSizes", bufferList);
        root->setProperty ("bufferSize", bufferSize);
        root->setProperty ("latencyMs", latencyMs);
        root->setProperty ("bufferMs", bufferMs);
        root->setProperty ("xruns", xruns);
        root->setProperty ("error", error);
        return juce::var (root);
    }
}
