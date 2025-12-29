#include "CustomLookAndFeel.h"

DualCoreLookAndFeel::DualCoreLookAndFeel()
{
    // Define color scheme
    accentColour = juce::Colour(0xff00aaff);      // Bright cyan/blue
    backgroundColour = juce::Colour(0xff1e1e35);  // Dark purple-grey
    darkColour = juce::Colour(0xff121220);        // Very dark
    lightColour = juce::Colour(0xff3a3a5a);       // Medium grey

    // Set default colors
    setColour(juce::Slider::rotarySliderFillColourId, accentColour);
    setColour(juce::Slider::rotarySliderOutlineColourId, lightColour);
    setColour(juce::Slider::thumbColourId, accentColour);
    setColour(juce::Slider::trackColourId, accentColour);
    setColour(juce::Slider::backgroundColourId, darkColour);

    setColour(juce::ComboBox::backgroundColourId, backgroundColour);
    setColour(juce::ComboBox::outlineColourId, lightColour);
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
    setColour(juce::ComboBox::arrowColourId, accentColour);

    setColour(juce::PopupMenu::backgroundColourId, backgroundColour);
    setColour(juce::PopupMenu::textColourId, juce::Colours::white);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accentColour.withAlpha(0.3f));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

    setColour(juce::TextButton::buttonColourId, backgroundColour);
    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    setColour(juce::TextButton::textColourOnId, accentColour);

    setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.95f));

    // Slider text box colors
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff1a1a30));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff3a3a5a));
}

void DualCoreLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                            juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(2.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Outer ring (background)
    g.setColour(darkColour);
    g.fillEllipse(rx, ry, rw, rw);

    // Outer ring border
    g.setColour(lightColour);
    g.drawEllipse(rx, ry, rw, rw, 1.5f);

    // Inner knob area
    float innerRadius = radius * 0.7f;
    float innerX = centreX - innerRadius;
    float innerY = centreY - innerRadius;
    float innerW = innerRadius * 2.0f;

    // Gradient for 3D effect
    juce::ColourGradient knobGradient(
        juce::Colour(0xff2a2a45), centreX, centreY - innerRadius,
        juce::Colour(0xff1a1a30), centreX, centreY + innerRadius, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(innerX, innerY, innerW, innerW);

    // Arc showing value
    juce::Path arcPath;
    float arcRadius = radius * 0.85f;
    float arcThickness = 3.0f;

    arcPath.addCentredArc(centreX, centreY, arcRadius, arcRadius,
                          0.0f, rotaryStartAngle, angle, true);

    g.setColour(accentColour);
    g.strokePath(arcPath, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // Indicator line
    juce::Path indicatorPath;
    float indicatorLength = radius * 0.5f;
    float indicatorStart = radius * 0.2f;

    indicatorPath.startNewSubPath(centreX + indicatorStart * std::sin(angle),
                                   centreY - indicatorStart * std::cos(angle));
    indicatorPath.lineTo(centreX + indicatorLength * std::sin(angle),
                         centreY - indicatorLength * std::cos(angle));

    g.setColour(accentColour);
    g.strokePath(indicatorPath, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

    // Center dot
    float dotRadius = 3.0f;
    g.setColour(accentColour.withAlpha(0.8f));
    g.fillEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
}

void DualCoreLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                            juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearHorizontal)
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        float trackHeight = 6.0f;
        float trackY = bounds.getCentreY() - trackHeight / 2.0f;

        // Background track
        g.setColour(darkColour);
        g.fillRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackHeight, 3.0f);

        // Border
        g.setColour(lightColour);
        g.drawRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackHeight, 3.0f, 1.0f);

        // Filled portion
        float fillWidth = sliderPos - bounds.getX();
        if (fillWidth > 0)
        {
            g.setColour(accentColour);
            g.fillRoundedRectangle(bounds.getX(), trackY, fillWidth, trackHeight, 3.0f);
        }

        // Thumb
        float thumbWidth = 12.0f;
        float thumbHeight = 16.0f;
        float thumbX = sliderPos - thumbWidth / 2.0f;
        float thumbY = bounds.getCentreY() - thumbHeight / 2.0f;

        g.setColour(juce::Colour(0xff2a2a45));
        g.fillRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 3.0f);

        g.setColour(accentColour);
        g.drawRoundedRectangle(thumbX, thumbY, thumbWidth, thumbHeight, 3.0f, 1.5f);
    }
    else
    {
        LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos,
                                          maxSliderPos, style, slider);
    }
}

void DualCoreLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto fontSize = juce::jmin(16.0f * currentScale, bounds.getHeight() * 0.65f);

    // Checkbox area
    float boxSize = bounds.getHeight() * 0.7f;
    float boxX = 4.0f;
    float boxY = (bounds.getHeight() - boxSize) / 2.0f;

    // Box background
    g.setColour(darkColour);
    g.fillRoundedRectangle(boxX, boxY, boxSize, boxSize, 3.0f);

    // Box border
    g.setColour(button.getToggleState() ? accentColour : lightColour);
    g.drawRoundedRectangle(boxX, boxY, boxSize, boxSize, 3.0f, 1.5f);

    // Checkmark or fill when toggled
    if (button.getToggleState())
    {
        g.setColour(accentColour);
        float padding = boxSize * 0.25f;
        g.fillRoundedRectangle(boxX + padding, boxY + padding,
                               boxSize - padding * 2, boxSize - padding * 2, 2.0f);
    }

    // Text
    g.setColour(button.getToggleState() ? accentColour : juce::Colours::white.withAlpha(0.9f));
    g.setFont(juce::FontOptions(fontSize));

    auto textBounds = bounds.withLeft(boxX + boxSize + 6.0f);
    g.drawText(button.getButtonText(), textBounds, juce::Justification::centredLeft);
}

void DualCoreLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                        int buttonX, int buttonY, int buttonW, int buttonH,
                                        juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();

    // Background
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border
    g.setColour(box.hasKeyboardFocus(true) ? accentColour : lightColour);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Arrow
    juce::Path arrow;
    float arrowX = width - 15.0f;
    float arrowY = height / 2.0f;
    float arrowSize = 5.0f;

    arrow.startNewSubPath(arrowX - arrowSize, arrowY - arrowSize / 2.0f);
    arrow.lineTo(arrowX, arrowY + arrowSize / 2.0f);
    arrow.lineTo(arrowX + arrowSize, arrowY - arrowSize / 2.0f);

    g.setColour(accentColour);
    g.strokePath(arrow, juce::PathStrokeType(1.5f));
}

void DualCoreLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                const juce::Colour& backgroundColour,
                                                bool shouldDrawButtonAsHighlighted,
                                                bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);

    auto baseColour = backgroundColour;
    if (shouldDrawButtonAsDown)
        baseColour = accentColour.withAlpha(0.5f);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);

    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(shouldDrawButtonAsHighlighted ? accentColour : lightColour);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

juce::Font DualCoreLookAndFeel::getComboBoxFont(juce::ComboBox& box)
{
    return juce::FontOptions(juce::jmin(18.0f * currentScale, static_cast<float>(box.getHeight()) * 0.8f));
}

juce::Font DualCoreLookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::FontOptions(juce::jmin(16.0f * currentScale, static_cast<float>(label.getHeight()) * 0.95f));
}

juce::Font DualCoreLookAndFeel::getPopupMenuFont()
{
    return juce::FontOptions(18.0f * currentScale);
}

juce::Font DualCoreLookAndFeel::getTextButtonFont(juce::TextButton& button, int buttonHeight)
{
    return juce::FontOptions(juce::jmin(17.0f * currentScale, static_cast<float>(buttonHeight) * 0.7f));
}

juce::Font DualCoreLookAndFeel::getAlertWindowTitleFont()
{
    return juce::FontOptions(22.0f * currentScale).withStyle("Bold");
}

juce::Font DualCoreLookAndFeel::getAlertWindowMessageFont()
{
    return juce::FontOptions(18.0f * currentScale);
}

juce::Font DualCoreLookAndFeel::getAlertWindowFont()
{
    return juce::FontOptions(18.0f * currentScale);
}

juce::Font DualCoreLookAndFeel::getSliderPopupFont(juce::Slider&)
{
    return juce::FontOptions(16.0f * currentScale);
}
