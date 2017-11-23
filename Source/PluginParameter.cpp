/*
 // Copyright (c) 2015-2017 Pierre Guillot.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
*/

#include "PluginParameter.h"
#include "PluginParser.h"
#include <cmath>

// ======================================================================================== //
//                                      PARAMETER                                           //
// ======================================================================================== //

CamomileAudioParameter::CamomileAudioParameter(const String& name, const String& label,
                                               const float min, const float max,
                                               const float def, const int nsteps,
                                               const bool automatable, const bool meta) :
m_name(name), m_label(label),
m_minimum(min), m_maximum(max),
m_default(def), m_nsteps(nsteps),
m_automatable(automatable), m_meta(meta) { m_value = getDefaultValue(); }

CamomileAudioParameter::CamomileAudioParameter(const String& name, const String& label,
                                               StringArray const& elems, const int def,
                                               const bool automatable, const bool meta) :
m_name(name), m_label(label),
m_minimum(0), m_maximum(elems.size() - 1),
m_default(static_cast<int>(def)), m_nsteps(elems.size()),
m_automatable(automatable), m_meta(meta),
m_elements(elems) { m_value = getDefaultValue(); }

CamomileAudioParameter::~CamomileAudioParameter() {}

String CamomileAudioParameter::getName(int maximumStringLength) const
{
    return m_name.substring(0, maximumStringLength);
}

String CamomileAudioParameter::getLabel() const
{
    return m_label;
}

float CamomileAudioParameter::getValue() const
{
    return m_value;
}

float CamomileAudioParameter::getOriginalScaledValue() const
{
    return m_value * (m_maximum - m_minimum) + m_minimum;
}

void CamomileAudioParameter::setValue(float newValue)
{
    if(isDiscrete())
    {
        m_value = round(newValue * static_cast<float>(m_maximum)) / static_cast<float>(m_maximum);
    }
    else
    {
        m_value = newValue;
    }
}

void CamomileAudioParameter::setOriginalScaledValue(float newValue)
{
    m_value = (newValue - m_minimum) / (m_maximum - m_minimum);
}

void CamomileAudioParameter::setOriginalScaledValueNotifyingHost(float newValue)
{
    setValueNotifyingHost((newValue - m_minimum) / (m_maximum - m_minimum));
}

float CamomileAudioParameter::getDefaultValue() const
{
    return (m_default - m_minimum) / (m_maximum - m_minimum);
}

int CamomileAudioParameter::getNumSteps() const
{
    return (m_nsteps > 0 && m_nsteps < 1e+37) ? m_nsteps : AudioProcessor::getDefaultNumParameterSteps();
}

bool CamomileAudioParameter::isDiscrete() const
{
    return (m_nsteps > 0 && m_nsteps < 1e+37);
}

String CamomileAudioParameter::getText(float value, int maximumStringLength) const
{
    if(m_elements.isEmpty())
    {
        return String(value * (m_maximum - m_minimum) + m_minimum).substring(0, maximumStringLength);
    }
    else
    {
        value = (value > 1.f) ? 1.f : value;
        value = (value < 0.f) ? 0.f : value;
        if(static_cast<int>(m_maximum)%2)
            return m_elements[static_cast<int>(floor(value * m_maximum))].substring(0, maximumStringLength);
        else
            return m_elements[static_cast<int>(ceil(value * m_maximum))].substring(0, maximumStringLength);
    }
}

float CamomileAudioParameter::getValueForText(const String& text) const
{
    if(m_elements.isEmpty())
    {
        return text.getFloatValue();
    }
    else
    {
        return static_cast<float>(m_elements.indexOf(text)) / static_cast<float>(m_maximum);
    }
}

bool CamomileAudioParameter::isOrientationInverted() const
{
    return m_minimum > m_maximum;
}

bool CamomileAudioParameter::isAutomatable() const
{
    return m_automatable;
}

bool CamomileAudioParameter::isMetaParameter() const
{
    return m_meta;
}

CamomileAudioParameter* CamomileAudioParameter::parse(const std::string& definition)
{
    auto options = CamomileParser::getOptions(definition);
    String const name = options.count("name") ? CamomileParser::getString(options["name"]) : "";
    String const label = options.count("min") ? CamomileParser::getString(options["label"]) : "";
    if(options.count("list"))
    {
        StringArray elems;
        auto const telems = CamomileParser::getList(options["list"]);
        for(auto const& el : telems) { elems.add(el); }
        const float def = options.count("default") ? CamomileParser::getFloat(options["default"]) : 0;
        const bool autom = options.count("auto") ? CamomileParser::getBool(options["auto"]) : true;
        const bool meta = options.count("meta") ? CamomileParser::getBool(options["meta"]) : false;
        return new CamomileAudioParameter(name, label, elems, def, autom, meta);
    }
    else
    {
        const float min = options.count("min") ? CamomileParser::getFloat(options["min"]) : 0;
        const float max = options.count("max") ? CamomileParser::getFloat(options["max"]) : 1;
        const float def = options.count("default") ? CamomileParser::getFloat(options["default"]) : min;
        const int nsteps = options.count("nsteps") ? CamomileParser::getInteger(options["nsteps"]) : 0;
        const bool autom = options.count("auto") ? CamomileParser::getBool(options["auto"]) : true;
        const bool meta = options.count("meta") ? CamomileParser::getBool(options["meta"]) : false;
        return new CamomileAudioParameter(name, label, min, max, def, nsteps, autom, meta);
    }
}

void CamomileAudioParameter::saveStateInformation(XmlElement& xml, OwnedArray<AudioProcessorParameter> const& parameters)
{
    XmlElement* params = xml.createNewChildElement("params");
    if(params)
    {
        for(int i = 0; i < parameters.size(); ++i)
        {
            params->setAttribute(String("param") + String(i+1), parameters[i]->getValue());
        }
    }
}

void CamomileAudioParameter::loadStateInformation(XmlElement const& xml, OwnedArray<AudioProcessorParameter> const& parameters)
{
    XmlElement const* params = xml.getChildByName(juce::StringRef("params"));
    if(params)
    {
        for(int i = 0; i < parameters.size(); ++i)
        {
            parameters[i]->setValue(params->getDoubleAttribute(String("param") + String(i+1), parameters[i]->getValue()));
        }
    }
}




