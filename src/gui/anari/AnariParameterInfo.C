// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnariParameterInfo.h>

// ****************************************************************************
// Method: AnariParameterInfo::AnariParameterInfo
//
// Purpose:
//   Move Constructor for the AnariParameterInfo class.
//
// Arguments:
//   other : another AnariParameterInfo object
//
// Programmer: Kevin Griffin
// Creation:   Mon Sep 23 14:46:40 PST 2002
//
// Modifications:
//
// ****************************************************************************
AnariParameterInfo::AnariParameterInfo(AnariParameterInfo &&other)
{
    m_name = other.m_name;
    m_type = other.m_type;
    other.m_type = ANARI_UNKNOWN;
    m_description = other.m_description;
    m_minimum = other.m_minimum;
    m_maximum = other.m_maximum;
    m_defaultValue = other.m_defaultValue;
    m_acceptedValues = other.m_acceptedValues;
}

// ****************************************************************************
// Method: AnariParameterInfo::operator
//
// Purpose:
//   Move Assignment Operator for the AnariParameterInfo class.
//
// Arguments:
//   other : another AnariParameterInfo object
//
// Programmer: Kevin Griffin
// Creation:   Mon Sep 23 14:46:40 PST 2002
//
// Modifications:
//
// ****************************************************************************
AnariParameterInfo &
AnariParameterInfo::operator=(AnariParameterInfo &&other)
{
    if (this != &other)
    {
        m_name = other.m_name;
        m_type = other.m_type;
        other.m_type = ANARI_UNKNOWN;
        m_description = other.m_description;
        m_minimum = other.m_minimum;
        m_maximum = other.m_maximum;
        m_defaultValue = other.m_defaultValue;
        m_acceptedValues = other.m_acceptedValues;
    }

    return *this;
}

void
AnariParameterInfo::SetDescription(const void *d)
{
    if(d)
    {
        m_description = std::string(reinterpret_cast<const char *>(d));
    }
    else
    {
        m_description.clear();
    }
}

void
AnariParameterInfo::SetAcceptedValues(const char **v)
{
    for (int i = 0; v && v[i] != nullptr; i++)
    {
        m_acceptedValues.push_back(v[i]);
    }
}
