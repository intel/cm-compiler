/*========================== begin_copyright_notice ============================

Copyright (C) 2009-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _INTEL_OPENCL_SAMPLE_CMDPARSER_HPP_
#define _INTEL_OPENCL_SAMPLE_CMDPARSER_HPP_


#include <vector>
#include <string>
#include <list>
#include <stdexcept>
#include <sstream>
#include <cassert>

#include "basic.hpp"

using std::string;
using std::vector;


// Base class for all options.
// We need some abstract mechanics here because all various
// options are delivered from a class template.
// So to handle all possible variants in CmdParser class (defined below
// in this file) in uniform way we need to define this basic class.
// Also this class defines common parsing rules and how the usage
// information is printed for an option
class CmdOptionBasic
{
    char   m_short_name;
    string m_long_name;
    string m_value_placeholder;
    string m_help_text;

    bool m_parsed;  // true if option was found in command line

public:

    CmdOptionBasic (
        char short_name,    // for example, 'o' for -o, pass 0 if not needed
        const string& long_name,    // for example, "option" for "--option", empty string if not needed
        const string& value_placeholder,
        const string& help_text
    );

    void setHelpText(const string& help_text)
    {
        m_help_text = help_text;
    }

    void setValuePlaceholder(const string& value_placeholder)
    {
        m_value_placeholder = value_placeholder;
    }

    string name () const;
    bool isSet () const;
    int parse (int cur_arg_index, int argc, const char** argv);

    bool printUsage (
        std::ostream& out,
        bool vertical_indent,
        size_t width = 79,
        size_t description_indent = 15
    ) const;

    virtual void finishParsing () = 0;

    // Automatic error message generation for invalid option value.
    void validate (
        bool status,    // true if option has a valid value, false otherwise
        const string& error_text = ""  // error text printed in case of fail
    ) const;

protected:

    // This function is used for parsing command-line arguments as objects of
    // target type, which is defined by a descendant class.
    virtual int parseValues (
        int cur_arg_index,
        const string& cur_arg_value,
        int argc,
        const char** argv
    ) = 0;

    virtual string defaultValueText () const = 0;
    virtual string enumValuesText () const = 0;
};


// Just a container for all options
class CmdParser
{
    int m_argc;
    const char** m_argv;
    std::list<CmdOptionBasic*> m_options;
    typedef std::list<CmdOptionBasic*>::iterator OptionIterator;
    typedef std::list<CmdOptionBasic*>::const_iterator OptionCIterator;

public:

    struct Error : public ::Error
    {
        Error (const string& msg) : ::Error(msg) {}
    };

    CmdParser (int argc, const char** argv);

    /// Add new option.
    /** There is no copy of a given option object in this call.
        A pointer to a given object is kept itself without any copying,
        so a caller should not destroy the option object till CmdParser dies. */
    void addOption (CmdOptionBasic* option);

    virtual void parse ();
    void printUsage (std::ostream& out) const;
};


// Forward declaration, see the definition below
template <typename T> class CmdOption;


// Represents one of the available options for CmdOption class.
template <typename T>
class CmdEnum
{
    T m_value;
    bool m_parsed;

public:

    // Initializes with "value" and registers in "option"
    CmdEnum (CmdOption<T>& option, const T& value);

    bool isSet () const
    {
        return m_parsed;
    }

    const T& getValue () const
    {
        return m_value;
    }

    bool parse (const T& x)
    {
        if(x == m_value)
        {
            m_parsed = true;
            return true;
        }
        return false;
    }
};


/*  Command-line option
    The following variants are supported:
        - simple switch without an argument, for example, -h
        - an option with one argument of specific type, for example, -p0, --platform 0
        - an option with one argument of specific type from a predefined list
            of alternatives, for example, --arithmetic float or --arithmetic double
    To make simple switch without an argument, just provide T = bool.
*/
template <typename T>
class CmdOption : public CmdOptionBasic
{
    T m_default_value;
    std::list<CmdEnum<T>*> m_alternatives;
    string m_default_text;
    T m_parsed_value;

public:

    CmdOption (
        CmdParser& cmd_parser,
        char short_name,    // short name, for example, 'o' for -o
        const string& long_name,    // long name, for example, "option" for --option
        const string& value_placeholder,    // text that is displayed in usage text, for example, "NUMBER" for --option NUMBER
        const string& help_text,    // long help text with description of option
        const T& default_value = T(),   // default value which is returned getValue if the option does not appear in the command line
        const string& default_text = ""    // default value to appear in usage info; if empty, default_value is used instead
    );

    void setDefaultValue(const T& default_value)
    {
        m_parsed_value = m_default_value = default_value;
    }
    const T& getValue () const
    {
        return m_parsed_value;
    }

    void addEnum (CmdEnum<T>* alternative)
    {
        m_alternatives.push_back(alternative);
    }

    virtual void finishParsing ();

protected:

    virtual int parseValues (
        int cur_arg_index,
        const string& cur_arg_value,
        int argc,
        const char** argv
    );

    virtual string defaultValueText () const;
    virtual string enumValuesText () const;
};


template <typename T>
CmdEnum<T>::CmdEnum (CmdOption<T>& option, const T& value) :
    m_value(value),
    m_parsed(false)
{
    option.addEnum(this);
}


template <typename T>
int CmdOption<T>::parseValues (
    int cur_arg_index,
    const string& cur_arg_value,
    int argc,
    const char** argv
)
{
    if(cur_arg_index >= argc)
    {
        throw CmdParser::Error(
            "Unexpected end of command line; a value expected."
        );
    }

    m_parsed_value = str_to<T>(cur_arg_value);

    // Check if a given value is valid
    if(!m_alternatives.empty())
    {
        bool found = false;
        for(
            typename std::list<CmdEnum<T>*>::iterator i = m_alternatives.begin();
            i != m_alternatives.end();
            ++i
        )
        {
            if((*i)->parse(m_parsed_value))
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            throw CmdParser::Error(
                "Illegal value for " + name() + " option: " +
                to_str(m_parsed_value)
            );
        }
    }

    return cur_arg_index + 1;
}

template <>
inline int CmdOption<bool>::parseValues (
    int cur_arg_index,
    const string& cur_arg_value,
    int argc,
    const char** argv
)
{
    m_parsed_value = !m_default_value;
    return cur_arg_index;
}


template <>
inline string CmdOption<bool>::defaultValueText () const
{
    return "";
}


template <typename T>
void CmdOption<T>::finishParsing ()
{
    if(!isSet() && !m_alternatives.empty())
    {
        // Mark the default alternative
        for(
            typename std::list<CmdEnum<T>*>::iterator i = m_alternatives.begin();
            i != m_alternatives.end();
            ++i
        )
        {
            if((*i)->parse(m_default_value))
            {
                // found first match and stop
                break;
            }
        }
    }
}


// Frequently used in samples set of command line options, which
// allows printing help, selecting platform and device type
// (but not particular device if there are multiple devices available).
class CmdParserDeviceType : public CmdParser
{
public:

    CmdParserDeviceType (int argc, const char** argv);

    CmdOption<bool> help;
    CmdOption<string> platform;
    CmdOption<string> device_type;
    CmdOption<string> input1_Filename;
    CmdOption<string> input2_Filename;
    CmdOption<string> output_Filename;
    CmdOption<string> alpha;

    virtual void parse ();
};


// Most commonly used set of command line options:
// besides options provided by CmdParserDeviceType, it also
// allows selection of particular device.
class CmdParserCommon : public CmdParserDeviceType
{
public:

    CmdParserCommon (int argc, const char** argv);
    CmdOption<string> device;
};


template <typename T>
CmdOption<T>::CmdOption (
    CmdParser& cmd_parser,
    char short_name,
    const string& long_name,
    const string& value_placeholder,
    const string& help_text,
    const T& default_value,
    const string& default_text
) :
    CmdOptionBasic(short_name, long_name, value_placeholder, help_text),
    m_default_value(default_value),
    m_default_text(default_text),
    m_parsed_value(default_value)
{
    // TODO: Support for vector types
    assert(m_alternatives.empty());
    cmd_parser.addOption(this);
}


template <typename T>
string CmdOption<T>::defaultValueText () const
{
    return m_default_text.empty() ? to_str(m_default_value) : m_default_text;
}


template <typename T>
string CmdOption<T>::enumValuesText () const
{
    if(m_alternatives.empty())
    {
        return "";
    }

    typename std::list<CmdEnum<T>*>::const_iterator i = m_alternatives.begin(), end = m_alternatives.end();
    string res = to_str((*i)->getValue());

    for(++i; i != end; ++i)
    {
        res += " | ";
        res += to_str((*i)->getValue());
    }

    return res;
}


class CmdOptionErrors : public CmdOption<int>
{
public:
    CmdOptionErrors (CmdParser& cmd_parser) :
        CmdOption<int>
        (
            cmd_parser,
            0,
            "errors",
            "<integer>",
            "Number of errors to print.",
            10
        )
    {}
};

class CmdOptionWorkGroupSize : public CmdOption<size_t>
{
public:
    CmdOptionWorkGroupSize (CmdParser& cmd_parser) :
        CmdOption<size_t>(
            cmd_parser,
            'g',
            "work-group-size",
            "<integer>",
            "OpenCL work group size. Set to 0 for work group size auto selection.",
            0
        )
    {}

    // Returns localSize value sutable for passing to clEnqueueNDRangeKernel.
    // It provides special processing for 0 value (returns null pointer, not pointer to zero).
    const size_t* localSize () const
    {
        return getValue() ? &getValue() : 0;
    }
};
#endif  // end of the include guard
