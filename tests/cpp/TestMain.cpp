#include <iostream>
#include <juce_events/juce_events.h>

struct ConsoleLogger : public juce::Logger
{
    void logMessage (const juce::String& message) override
    {
        std::cout << message << std::endl;
    }
};

int main()
{
    juce::ScopedJuceInitialiser_GUI initialiser;
    ConsoleLogger logger;
    juce::Logger::setCurrentLogger (&logger);

    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.runAllTests();

    int failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    juce::Logger::writeToLog (failures == 0 ? "ALL TESTS PASSED" : "TESTS FAILED: " + juce::String (failures));
    juce::Logger::setCurrentLogger (nullptr);
    return failures == 0 ? 0 : 1;
}
