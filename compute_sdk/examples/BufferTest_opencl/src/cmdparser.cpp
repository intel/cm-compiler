/*========================== begin_copyright_notice ============================

Copyright (C) 2009-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cassert>
#include <iostream>
#include <iomanip>

#include "cmdparser.hpp"


CmdOptionBasic::CmdOptionBasic (
    char short_name,    // pass 0 if not needed
    const string& long_name,
    const string& value_placeholder,
    const string& help_text
) :
    m_short_name(short_name),
    m_long_name(long_name),
    m_value_placeholder(value_placeholder),
    m_help_text(help_text),
    m_parsed(false)
{
    string illegal_symbols = " \t\n<>\"'\\";

    if(
        illegal_symbols.find(m_short_name) != string::npos ||
        m_short_name == '-')
    {
        throw CmdParser::Error(
            string("Illegal symbol is used in option name \"") +
            m_short_name + "\"\n"
        );
    }

    if(m_long_name.find_first_of(illegal_symbols) != string::npos)
    {
        throw CmdParser::Error(
            "Illegal symbol is used in option name \"" +
            m_long_name + "\"\n"
        );
    }
}


string CmdOptionBasic::name () const
{
    return m_long_name.empty() ? string("-") + m_short_name : "--" + m_long_name;
}


bool CmdOptionBasic::isSet () const
{
    return m_parsed;
}


int CmdOptionBasic::parse (int cur_arg_index, int argc, const char** argv)
{
    // Parse option name first, then rely on inherited classes
    // to parse value(s)

    if(cur_arg_index >= argc)
    {
        // out of command line arguments array
        return cur_arg_index;
    }

    string cur_arg_value = argv[cur_arg_index];

    if(cur_arg_value.length() <= 1)
    {
        // option name cannot be one symbol or empty;
        // it must contain at least two symbols (for example, -h)
        return cur_arg_index;
    }

    if(
        // check for short name
        m_short_name != 0 &&
        cur_arg_value[0] == '-' &&
        cur_arg_value[1] == m_short_name ||
        // check for long name
        cur_arg_value == "--" + m_long_name
    )
    {
        if(m_parsed)
        {
            // option duplication is not allowed
            throw CmdParser::Error(
                "Option duplication: " + cur_arg_value
            );
        }

        m_parsed = true;
        // OK, we found option name, for example -o or --option
        // now determine if a value is splitted or not, for example -o value or -ovalue
        if(
            cur_arg_value.length() == 2 ||  // two symbols, for example it is only -o
            cur_arg_value == "--" + m_long_name // long name is always splitted --option value
        )
        {
            // it is splitted, move to next command line argument in the list
            return parseValues(
                cur_arg_index + 1,
                cur_arg_index + 1 == argc ? "" : argv[cur_arg_index + 1],
                argc,
                argv
            );
        }
        else
        {
            // it sticks with option name, -ovalue
            return parseValues(
                cur_arg_index,
                cur_arg_value.substr(2),
                argc,
                argv
            );
        }
    }

    // cannot recognize this option
    return cur_arg_index;
}


bool CmdOptionBasic::printUsage (
    std::ostream& out,
    bool vertical_indent,
    size_t width,
    size_t description_indent
) const
{
    const size_t min_middle_indent = 2;
    if(width < description_indent)
    {
        width = description_indent;
    }

    std::ostringstream buf;

    if(m_short_name != 0)
    {
        buf << '-' << m_short_name;
    }
    else
    {
        buf << "    ";
    }

    if(!m_long_name.empty())
    {
        if(m_short_name != 0)
        {
            buf << ", ";
        }

        buf << "--" << m_long_name;
    }

    if(m_value_placeholder.empty())
    {
        string enum_values_text = enumValuesText();
        if(!enum_values_text.empty())
        {
            buf << " " << enum_values_text;
        }
    }
    else
    {
        buf << " " << m_value_placeholder;
    }

    string header = buf.str();
    buf.clear();

    // Build full help text as one line:
    string full_help = m_help_text;

    string default_value = defaultValueText();
    if(!default_value.empty())
    {
        full_help += " (Default value: " + default_value + ")";
    }

    if(
        header.length() + min_middle_indent > description_indent ||
        full_help.length() > width - description_indent
    )
    {
        // Multi-line form
        if(!vertical_indent)
        {
            out << "\n";
        }

        width -= description_indent;

        out << header;
        string indent = string(description_indent, ' ');

        if(header.length() + min_middle_indent > description_indent)
        {
            out << "\n" << indent;
        }
        else
        {
            out << string(description_indent - header.length(), ' ');
        }

        bool first_row = true;

        while(!full_help.empty())
        {
            // erase leading spaces
            full_help.erase(0, full_help.find_first_not_of(' '));

            if(full_help.empty())
            {
                break;
            }

            if(!first_row)
            {
                out << indent;
            }

            first_row = false;

            size_t div = full_help.length();
            if(div > width)
            {
                div = full_help.substr(0, width).find_last_of(' ');
            }

            if(div == string::npos)
            {
                // formating failure, go out of width
                // OR just end of text
                if(full_help.length() < width)
                {
                    div = full_help.length();
                }
                else
                {
                    div = full_help.find_first_of(' ');
                }
            }

            out << full_help.substr(0, div) << "\n";
            if(div == string::npos)
            {
                full_help.erase();
            }
            else
            {
                full_help.erase(0, div + 1);    // erase with space
            }
        }

        return true;
    }
    else
    {
        // Single-line form
        out << header << string(description_indent - header.length(), ' ') << full_help << "\n";
        return false;
    }
}


void CmdOptionBasic::validate (
    bool status,
    const string& error_text
) const
{
    if(status)return;

    CmdParser::Error error(
        string("Illegal value for ") + name() + " parameter" +
        (error_text.empty() ? string("") : ": " + error_text) + "."
    );

    throw error;
}


CmdParser::CmdParser (int argc, const char** argv) :
    m_argc(argc),
    m_argv(argv)
{
}


void CmdParser::addOption (CmdOptionBasic* option)
{
    m_options.push_back(option);
}


void CmdParser::parse ()
{
    // bruteforce
    for(
        int cur_arg_index = 1;
        cur_arg_index < m_argc;
        /* do not increment; incremented in an option parse call */
    )
    {
        int old_cur_arg_index = cur_arg_index;
        for(
            OptionIterator i = m_options.begin();
            i != m_options.end();
            ++i
        )
        {
            cur_arg_index = (*i)->parse(cur_arg_index, m_argc, m_argv);
            if(cur_arg_index > old_cur_arg_index)
            {
                break;
            }
        }

        if(cur_arg_index == old_cur_arg_index)
        {
            // no option can recognize the current argument
            throw CmdParser::Error(
                string("Unrecognized option or unexpected token: ") +
                m_argv[cur_arg_index]
            );
        }
    }

    for(
        OptionIterator i = m_options.begin();
        i != m_options.end();
        ++i
    )
    {
        (*i)->finishParsing();
    }
}


void CmdParser::printUsage (std::ostream& out) const
{
    out << "Usage:\n" << m_argv[0];
    if(!m_options.empty())
    {
        out << " [OPTIONS]";
    }
    else
    {
        return;
    }

    out << "\n\nAvailable options:\n\n";

    OptionCIterator i = m_options.begin();
    bool vertical_indent = (*i)->printUsage(out, false);
    ++i;

    for(
        ;
        i != m_options.end();
        ++i
    )
    {
        if(vertical_indent)
        {
            out << "\n";
        }

        vertical_indent = (*i)->printUsage(out, vertical_indent);
    }
    out << "\n";
}


#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4355)    // 'this': used in base member initializer list
#endif

CmdParserDeviceType::CmdParserDeviceType (int argc, const char** argv) :
    CmdParser(argc, argv),
    help(
        *this,
        'h',
        "help",
        "",
        "Show this help text and exit."
    ),
    platform(
        *this,
        'p',
        "platform",
        "<number-or-string>",
        "Selects the platform, the devices of which are used.",
        "Intel"
    ),
    device_type(
        *this,
        't',
        "type",
        "all | cpu | gpu | acc | default | <OpenCL constant for device type>",
        "Selects the device by type on which the OpenCL kernel is executed.",
        "all"
    ),
    input1_Filename(
        *this,
        'i',
        "infile",
        "24/32-bit .bmp file",
        "Base name of the .bmp file to read",
        "images\\input1.bmp" ),
    input2_Filename(
        *this,
        'j',
        "infile",
        "24/32-bit .bmp file",
        "Base name of the .bmp file to read",
        "images\\input2.bmp" ),
    output_Filename(
        *this,
        'o',
        "outfile",
        "24/32-bit bmp file",
        "Base name of the .bmp file to read",
        "images\\Output.bmp" ),
    alpha(
        *this,
        'a',
        "alpha",
        "floating Value between 0 and 1",
        "non-zero positive floating point value of alpha used for calculation",
        "0.84089642" )
{
}

CmdParserCommon::CmdParserCommon(int argc, const char** argv) :
    CmdParserDeviceType(argc, argv),
    device(
        *this,
        'd',
        "device",
        "<number-or-string>",
        "Selects the device on which all stuff is executed.",
        "0"
    )
{
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif


void CmdParserDeviceType::parse ()
{
    CmdParser::parse();
    if(help.isSet())
    {
        printUsage(std::cout);
    }
}
