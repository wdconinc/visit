// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <AnariRenderingWidget.h>
#include <AnariParameterInfo.h>
#include <QvisRenderingWindow.h>
#include <RenderingAttributes.h>
#include <DebugStream.h>

#include <QGroupBox>
#include <QComboBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QString>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QCheckBox>
#include <QSpacerItem>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>

#include <algorithm>

namespace anari_visit
{
    void StatusCallback(const void* userData, anari::Device device,
                        anari::Object source, anari::DataType sourceType, anari::StatusSeverity severity,
                        anari::StatusCode code, const char* message)
    {
        std::cerr << message << std::endl;
        // if (severity == ANARI_SEVERITY_FATAL_ERROR)
        // {
        //     std::cout << "[ANARI::FATAL] " << message;
        // }
        // else if (severity == ANARI_SEVERITY_ERROR)
        // {
        //     std::cout << "[ANARI::ERROR] " << %s, DataType: %d\n", message, (int)sourceType);
        // }
        // else if (severity == ANARI_SEVERITY_WARNING)
        // {
        //     std::cout(WARNING, "[ANARI::WARN] %s, DataType: %d\n", message, (int)sourceType);
        // }
        // else if (severity == ANARI_SEVERITY_PERFORMANCE_WARNING)
        // {
        //     std::cout(WARNING, "[ANARI::PERF] %s\n", message);
        // }
        // else if (severity == ANARI_SEVERITY_INFO)
        // {
        //     std::cout(INFO, "[ANARI::INFO] %s\n", message);
        // }
        // else if (severity == ANARI_SEVERITY_DEBUG)
        // {
        //     std::cout(TRACE, "[ANARI::DEBUG] %s\n", message);
        // }
        // else
        // {
        //     std::cout(INFO, "[ANARI::STATUS] %s\n", message);
        // }
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::AnariRenderingWidget
//
// Purpose:
//   Constructor for the AnariRenderingWidget class.
//
// Arguments:
//   qrw        Window that displays rendering settings
//   ra         Contains ANARI rendering attributes
//   parent     If parent is another widget, this widget becomes a
//              child window inside parent. The new widget is deleted
//                          when its parent is deleted.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

AnariRenderingWidget::AnariRenderingWidget(QvisRenderingWindow *qrw,
                                           RenderingAttributes *ra,
                                           QWidget *parent)
    : QWidget(parent)
    , renderingWindow(qrw)
    , renderingAttributes(ra)
    , dynamicLayouts(nullptr)
    , dynamicLayoutMap()
    , totalRows(0)
    , renderingGroup(nullptr)
    , libraryName(nullptr)
    , librarySubtypes(nullptr)
    , rendererSubtypes(nullptr)
{
    // row, col, rowspan, colspan
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // Rendering Group
    renderingGroup = new QGroupBox(tr("ANARI Rendering"));
    renderingGroup->setCheckable(true);
    renderingGroup->setChecked(false);
    connect(renderingGroup, &QGroupBox::toggled,
            this, &AnariRenderingWidget::renderingToggled);

    QVBoxLayout *renderingGroupVBoxLayout = new QVBoxLayout(renderingGroup);
    int rows =  totalRows;

    renderingGroupVBoxLayout->addWidget(CreateGeneralWidget(rows));
    totalRows += rows + 1;

    dynamicLayouts = new QStackedLayout();
    dynamicLayouts->addWidget(new QWidget(this)); // Placeholder for index 0

    // Create and add the back-end specific widgets
    // int rows1 = 0;
    // backendStackedLayout->addWidget(CreateBackendWidget(rows1));

    // // Create USD back-end widgets
    // int rows2 = 0;
    // backendStackedLayout->addWidget(CreateUSDWidget(rows2));

    // totalRows += std::max(rows1, rows2);

    renderingGroupVBoxLayout->addLayout(dynamicLayouts);
    mainLayout->addWidget(renderingGroup);

    connect(this, &AnariRenderingWidget::currentBackendChanged,
            dynamicLayouts, &QStackedLayout::setCurrentIndex);
}

// ****************************************************************************
// Method: AnariRenderingWidget::CreateGeneralWidget
//
// Purpose:
//   Creates the UI components for selecting back-end options used by all
//   ANARI libraries.
//
// Arguments:
//   rows keeps track of the total rows used to create this widget
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

QWidget *
AnariRenderingWidget::CreateGeneralWidget(int &rows)
{
    QWidget *generalOptionsWidget = new QWidget(this);

    QGridLayout *gridLayout = new QGridLayout(generalOptionsWidget);
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(10,10,10,10);

    gridLayout->setColumnStretch(1, 2);
    gridLayout->setColumnStretch(3, 2);
    gridLayout->setColumnStretch(4, 5);

    libraryName = new QLineEdit("", generalOptionsWidget);
    connect(libraryName, &QLineEdit::editingFinished,
            this, &AnariRenderingWidget::libraryChanged);

    // Back-end and subtype
    QLabel *backendLabel = new QLabel(tr("Back-end"));
    backendLabel->setToolTip(tr("ANARI back-end device"));

    gridLayout->addWidget(backendLabel, rows, 0, 1, 1);
    gridLayout->addWidget(libraryName, rows, 1, 1, 2);

    // Back-end subtype
    librarySubtypes = new QComboBox();
    librarySubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(librarySubtypes, &QComboBox::currentTextChanged,
            this, &AnariRenderingWidget::librarySubtypeChanged);

    QLabel *subtypeLabel = new QLabel(tr("Back-end Subtype"));

    gridLayout->addWidget(subtypeLabel, rows, 3, 1, 1);
    gridLayout->addWidget(librarySubtypes, rows, 4, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 4, 1, 1);

    // Renderer
    rendererSubtypes = new QComboBox();
    rendererSubtypes->setInsertPolicy(QComboBox::InsertPolicy::InsertAlphabetically);
    connect(rendererSubtypes, &QComboBox::currentTextChanged,
            this, &AnariRenderingWidget::rendererSubtypeChanged);

    QLabel *rendererLabel = new QLabel(tr("Renderer"));
    rendererLabel->setToolTip(tr("Renderer subtype"));

    gridLayout->addWidget(rendererLabel, rows, 0, 1, 1);
    gridLayout->addWidget(rendererSubtypes, rows, 1, 1, 1);

    gridLayout->addItem(new QSpacerItem(10, 10), rows++, 3, 1, 3);

    return generalOptionsWidget;
}

QWidget *
AnariRenderingWidget::MakeWidgetFromParameterInfo(const AnariParameterInfo &paramInfo)
{
    if(paramInfo.GetName() == "name" || paramInfo.GetName() == "background")
    {
        return nullptr;
    }

    switch(paramInfo.GetType())
    {
        case ANARI_INT32:
        {
            QSpinBox *spinBox = new QSpinBox();
            spinBox->setObjectName(paramInfo.GetName().c_str());

            if(paramInfo.m_defaultValue)
            {
                auto intPtr = static_cast<const int *>(paramInfo.m_defaultValue);
                spinBox->setValue(*intPtr);
            }

            if(paramInfo.HasMinimum())
            {
                auto minPtr = static_cast<const int *>(paramInfo.m_minimum);
                spinBox->setMinimum(*minPtr);
            }
            else
            {
                spinBox->setMinimum(std::numeric_limits<int>::min());
            }

            if(paramInfo.HasMaximum())
            {
                auto maxPtr = static_cast<const int *>(paramInfo.m_maximum);
                spinBox->setMaximum(*maxPtr);
            }
            else
            {
                spinBox->setMaximum(std::numeric_limits<int>::max());
            }

            connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, &AnariRenderingWidget::spinBoxValueChanged);

            return spinBox;
        }
        case ANARI_FLOAT32:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            float min = std::numeric_limits<float>::min();
            float max = std::numeric_limits<float>::max();

            if(paramInfo.m_defaultValue)
            {
                auto floatPtr = static_cast<const float *>(paramInfo.m_defaultValue);
                lineEdit->setText(QString::number(*floatPtr));
            }

            if(paramInfo.HasMinimum())
            {
                auto minPtr = static_cast<const float *>(paramInfo.m_minimum);
                min = *minPtr;
            }

            if(paramInfo.HasMaximum())
            {
                auto maxPtr = static_cast<const float *>(paramInfo.m_maximum);
                max = *maxPtr;
            }

            QDoubleValidator *dv = new QDoubleValidator(min, max, 4);
            lineEdit->setValidator(dv);

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
        case ANARI_FLOAT64:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            double min = std::numeric_limits<double>::min();
            double max = std::numeric_limits<double>::max();

            if(paramInfo.m_defaultValue)
            {
                auto floatPtr = static_cast<const double *>(paramInfo.m_defaultValue);
                lineEdit->setText(QString::number(*floatPtr));
            }

            if(paramInfo.HasMinimum())
            {
                auto minPtr = static_cast<const double *>(paramInfo.m_minimum);
                min = *minPtr;
            }

            if(paramInfo.HasMaximum())
            {
                auto maxPtr = static_cast<const double *>(paramInfo.m_maximum);
                max = *maxPtr;
            }

            QDoubleValidator *dv = new QDoubleValidator(min, max, 4);
            lineEdit->setValidator(dv);

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);

            return lineEdit;
        }
        case ANARI_STRING:
        {
            auto acceptedVals = paramInfo.GetAcceptedValues();

            if(!acceptedVals.empty())
            {
                QComboBox *comboBox = new QComboBox();
                comboBox->setObjectName(paramInfo.GetName().c_str());

                for(const std::string &value : acceptedVals)
                {
                    comboBox->addItem(value.c_str());
                }

                connect(comboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
                        this, &AnariRenderingWidget::comboBoxTextChanged);
                return comboBox;
            }

            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            if(paramInfo.m_defaultValue)
            {
                lineEdit->setText(static_cast<const char *>(paramInfo.m_defaultValue));
            }

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);
            return lineEdit;
        }
        case ANARI_BOOL:
        {
            QCheckBox *checkBox = new QCheckBox(paramInfo.GetName().c_str());
            checkBox->setObjectName(paramInfo.GetName().c_str());

            std::string toolTip = paramInfo.GetDescription();

            if(!toolTip.empty())
            {
                checkBox->setToolTip(tr(toolTip.c_str()));
            }

            if(paramInfo.m_defaultValue)
            {
                auto boolPtr = static_cast<const bool *>(paramInfo.m_defaultValue);
                checkBox->setChecked(*boolPtr);
            }

            connect(checkBox, &QCheckBox::toggled,
                    this, &AnariRenderingWidget::checkBoxToggled);
            return checkBox;
        }
        default:
        {
            QLineEdit *lineEdit = new QLineEdit();
            lineEdit->setObjectName(paramInfo.GetName().c_str());

            connect(lineEdit, &QLineEdit::editingFinished,
                    this, &AnariRenderingWidget::lineEditingFinished);
            return lineEdit;
        }
    }
}

void
AnariRenderingWidget::CreateDynamicWidget(anari::Device anariDevice, const char *subtype, const std::string &key, bool isUSD)
{
    int stackLayoutIndex = 0;
    auto resultIter = dynamicLayoutMap.find(key);

    if(resultIter == dynamicLayoutMap.end())
    {
        std::cout << "Creating dynamic widget for " << key << std::endl;
        if(!isUSD)
        {
            QWidget *dynamicWidget = new QWidget(this);

            QGridLayout *gridLayout = new QGridLayout(dynamicWidget);
            gridLayout->setSpacing(10);
            gridLayout->setContentsMargins(10,10,10,10);

            int rows = 0;
            int cols = 0;

            const ANARIParameter *parameterList =
                static_cast<const ANARIParameter*>(anariGetObjectInfo(anariDevice,
                                                                     ANARI_RENDERER,
                                                                     subtype,
                                                                     "parameter",
                                                                     ANARI_PARAMETER_LIST));

            for(const ANARIParameter *param = parameterList; param && param->name != nullptr; ++param)
            {
                AnariParameterInfo paramInfo = std::move(GetParameterInfo(anariDevice, ANARI_RENDERER, subtype, param));

                // Create the UI
                QWidget *layoutWidget = MakeWidgetFromParameterInfo(paramInfo);

                if(layoutWidget == nullptr)
                {
                    continue;
                }

                if(paramInfo.GetType() != ANARI_BOOL)
                {
                    QLabel *label = new QLabel(param->name);
                    std::string toolTip = paramInfo.GetDescription();

                    if(!toolTip.empty())
                    {
                        label->setToolTip(tr(toolTip.c_str()));
                    }

                    gridLayout->addWidget(label, rows, cols++, 1, 1);
                    gridLayout->addWidget(layoutWidget, rows, cols++, 1, 1);
                }
                else
                {
                    gridLayout->addWidget(layoutWidget, rows, cols, 1, 2);
                    cols += 2;
                }

                // Update rows and columns
                cols %= 4;

                if (cols == 0)
                {
                    ++rows;
                }
            }

            stackLayoutIndex = dynamicLayouts->addWidget(dynamicWidget);
        }
        else
        {
            stackLayoutIndex = 0; // TODO: stackLayoutIndex = dynamicLayouts->addWidget(CreateUSDWidget(stackLayoutIndex));
        }

        dynamicLayoutMap[key] = stackLayoutIndex;
    }
    else
    {
        std::cout << "Dynamic widget already exists for " << key << std::endl;
        stackLayoutIndex = resultIter->second;
    }

    emit currentBackendChanged(stackLayoutIndex);
}

// ****************************************************************************
// Method: AnariRenderingWidget::GetBackendType
//
// Purpose:
//   Gets the back-end type represented by libname.
//
// Arguments:
//   libname the name of the back-end to load
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

BackendType
AnariRenderingWidget::GetBackendType(const std::string &libname) const
{
    if(libname == "helide")
    {
        return BackendType::EXAMPLE;
    }
    else if(libname == "usd")
    {
        return BackendType::USD;
    }
    else if(libname == "visrtx")
    {
        return BackendType::VISRTX;
    }
    else if(libname == "visgl")
    {
        return BackendType::VISGL;
    }
    else if(libname == "ospray")
    {
        return BackendType::OSPRAY;
    }
     else if(libname == "rpr")
    {
        return BackendType::RADEONPRORENDER;
    }

    return BackendType::NONE;
}

AnariParameterInfo
AnariRenderingWidget::GetParameterInfo(anari::Device device,
                                       ANARIDataType objectType,
                                       const char *objectSubtype,
                                       const ANARIParameter *param)
{
    AnariParameterInfo paramInfo;

    paramInfo.SetName(param->name);
    paramInfo.SetType(param->type);

    // Explanation of the parameter, e.g., for a tooltip
    paramInfo.SetDescription(anariGetParameterInfo(device,
                                                   objectType,
                                                   objectSubtype,
                                                   param->name,
                                                   param->type,
                                                   "description",
                                                   ANARI_STRING));
    // set values will be clamped to this minimum
    paramInfo.m_minimum = anariGetParameterInfo(device,
                                                objectType,
                                                objectSubtype,
                                                param->name,
                                                param->type,
                                                "minimum",
                                                param->type);
    // set values will be clamped to this maximum
    paramInfo.m_maximum = anariGetParameterInfo(device,
                                                objectType,
                                                objectSubtype,
                                                param->name,
                                                param->type,
                                                "maximum",
                                                param->type);
    // default value, must be in minumum and maximum if present
    paramInfo.m_defaultValue = anariGetParameterInfo(device,
                                                     objectType,
                                                     objectSubtype,
                                                     param->name,
                                                     param->type,
                                                     "default",
                                                     param->type);
    // list of accepted values
    paramInfo.SetAcceptedValues((const char **)anariGetParameterInfo(device,
                                                                     objectType,
                                                                     objectSubtype,
                                                                     param->name,
                                                                     param->type,
                                                                     "value",
                                                                     ANARI_STRING_LIST));

    return paramInfo;
}

// External Updates
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibrarySubtypes
//
// Purpose:
//   Adds subtype to the library subtypes combo box. If subtype is already in
//   the list, it will be ignored.
//
// Arguments:
//   subtype the library subtype to add to the combo box
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLibrarySubtypes(const std::string subtype)
{
    librarySubtypes->blockSignals(true);
    QString textItem = QString::fromStdString(subtype);
    int index =  librarySubtypes->findText(textItem);

    if(index == -1)
    {
        librarySubtypes->addItem(textItem);
    }

    librarySubtypes->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateLibraryName
//
// Purpose:
//   Updates the available ANARI back-end.
//
// Arguments:
//   libname the name of the ANARI back-end
//
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateLibraryName(const std::string libname)
{
    libraryName->blockSignals(true);
    libraryName->setText(QString::fromStdString(libname));
    libraryName->blockSignals(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::UpdateRendererSubtypes
//
// Purpose:
//   Updates the list of available renderers. If subtype is already in the list
//   it will not be added again.
//
// Arguments:
//   subtype the renderer subtype to add
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::UpdateRendererSubtypes(const std::string subtype)
{
    rendererSubtypes->blockSignals(true);
    QString textItem = QString::fromStdString(subtype);
    int index =  rendererSubtypes->findText(textItem);

    if(index == -1)
    {
        rendererSubtypes->addItem(textItem);
    }

    rendererSubtypes->blockSignals(false);
}

void
AnariRenderingWidget::UpdateRendererParams(const stringVector &params)
{
    // TODO: Implement
    // 1. Get the current dynamic widget
    // 2. Update each widget matching name with the new value
}

// ****************************************************************************
// Method: AnariRenderingWidget::SetChecked
//
// Purpose:
//   Sets the check state of the ANARI rendering group box.
//
// Arguments:
//   val    If true, surface rendering will be done by an ANARI back-end
//          renderer, otherwise, the default rendering is used.
//
// Programmer: Kevin Griffin
// Creation:
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::SetChecked(const bool val)
{
    renderingGroup->setChecked(val);
}

// SLOTS
//----------------------------------------------------------------------------

// ****************************************************************************
// Method: AnariRenderingWidget::renderingToggled
//
// Purpose:
//      Triggered when ANARI rendering is toggled.
//
// Arguments:
//      val when true use ANARI for rendering
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::renderingToggled(bool val)
{
    renderingAttributes->SetAnariRendering(val);
    renderingWindow->SetUpdateApply(false);
}

// ****************************************************************************
// Method: AnariRenderingWidget::libraryChanged
//
// Purpose:
//      Triggered when ANARI Back-end rendering library has changed.
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::libraryChanged()
{
    renderingAttributes->SetUsingUsdDevice(false);
    auto libname = libraryName->text().trimmed().toStdString(); // .c_str();
    auto anariLibrary = anari::loadLibrary(libname.c_str(), anari_visit::StatusCallback);

    if(anariLibrary)
    {
        renderingAttributes->SetAnariLibrary(libname);
        auto backendType = GetBackendType(libraryName->text().trimmed().toStdString());

        if(backendType == BackendType::USD)
        {
            renderingAttributes->SetUsingUsdDevice(true);
        }
        else
        {
            renderingAttributes->SetUsingUsdDevice(false);
        }

        // Update back-end subtypes
        librarySubtypes->blockSignals(true);
        librarySubtypes->clear();
        const char **devices = anariGetDeviceSubtypes(anariLibrary);

        if(devices)
        {
            for(const char **d = devices; *d != NULL; d++)
            {
                librarySubtypes->addItem(*d);
            }
        }
        else
        {
            librarySubtypes->addItem("default");
        }

        librarySubtypes->blockSignals(false);
        auto libSubtype =  librarySubtypes->currentText().toStdString();
        renderingAttributes->SetAnariLibrarySubtype(libSubtype);

        auto anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());

        // Update renderers
        rendererSubtypes->blockSignals(true);
        rendererSubtypes->clear();

        const char **renderers = anariGetObjectSubtypes(anariDevice, ANARI_RENDERER);

        if(renderers)
        {
            for(const char **d = renderers; *d != NULL; d++)
            {
                rendererSubtypes->addItem(*d);
            }
        }
        else
        {
            rendererSubtypes->addItem("default");
        }

        auto rendererSubtype = rendererSubtypes->currentText().toStdString();
        renderingAttributes->SetAnariRendererSubtype(rendererSubtype);
        rendererSubtypes->blockSignals(false);

        // Create Dynamic Widget
        std::string key = libname + ":" + libSubtype + ":" + rendererSubtype;
        CreateDynamicWidget(anariDevice, rendererSubtype.c_str(), key, backendType == BackendType::USD);

        // Clean-up
        anari::release(anariDevice, anariDevice);
        anariUnloadLibrary(anariLibrary);

        renderingWindow->SetUpdateApply(false);
    }
    else
    {
        QString message;

        if(libraryName->text().trimmed() == "environment")
        {
            message.append(tr("ANARI_LIBRARY not set."));
        }
        else
        {
            message.append(tr("%1 is not a valid back-end name or not on your library path.").arg(libname.c_str()));
        }

        QMessageBox::critical(this, tr("ANARI"), message);
        debug1 << "Could not load the ANARI library (" << libname << ") to update the Rendering UI." << std::endl;

        // Reset Back-end Subtype and Renderer to "default"
        librarySubtypes->blockSignals(true);
        librarySubtypes->clear();
        librarySubtypes->addItem("default");
        librarySubtypes->blockSignals(false);
        auto libSubtype =  librarySubtypes->currentText().toStdString();
        renderingAttributes->SetAnariLibrarySubtype(libSubtype);

        rendererSubtypes->blockSignals(true);
        rendererSubtypes->clear();
        rendererSubtypes->addItem("default");
        rendererSubtypes->blockSignals(false);
        auto rendererSubtype = rendererSubtypes->currentText().toStdString();
        renderingAttributes->SetAnariRendererSubtype(rendererSubtype);

        // Reset to blank widget
        emit currentBackendChanged(0);
        renderingWindow->SetUpdateApply(false);
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::librarySubtypeChanged
//
// Purpose:
//      Triggered when ANARI Library subtype has changed.
//
// Arguments:
//      subtype the new library subtype
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::librarySubtypeChanged(const QString &subtype)
{
    auto libSubtype = subtype.toStdString();
    renderingAttributes->SetAnariLibrarySubtype(libSubtype);
    auto libname = libraryName->text().trimmed().toStdString();

    auto anariLibrary = anari::loadLibrary(libname.c_str(), anari_visit::StatusCallback);
    auto anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());

    if(anariDevice)
    {
        // Update renderers
        rendererSubtypes->blockSignals(true);
        rendererSubtypes->clear();
        const char **renderers = anariGetObjectSubtypes(anariDevice, ANARI_RENDERER);

        if(renderers)
        {
            for(const char **d = renderers; *d != NULL; d++)
            {
                rendererSubtypes->addItem(*d);
            }
        }
        else
        {
            rendererSubtypes->addItem("default");
        }

        auto rendererSubtype =  rendererSubtypes->currentText().toStdString();
        renderingAttributes->SetAnariRendererSubtype(rendererSubtype);
        rendererSubtypes->blockSignals(false);

        // Create Dynamic Widget
        std::string key = libname + ":" + libSubtype + ":" + rendererSubtype;
        CreateDynamicWidget(anariDevice, rendererSubtype.c_str(), key, GetBackendType(libname) == BackendType::USD);

        // Clean-up
        anari::release(anariDevice, anariDevice);
        anariUnloadLibrary(anariLibrary);

        renderingWindow->SetUpdateApply(false);
    }
    else
    {
        debug1 << "Could not create the ANARI back-end device (" << libname << ") to update the Rendering UI." << std::endl;
        emit currentBackendChanged(0);
        renderingWindow->SetUpdateApply(false);
    }
}

// ****************************************************************************
// Method: AnariRenderingWidget::rendererSubtypeChanged
//
// Purpose:
//      Triggered when ANARI renderer subtype has changed.
//
// Arguments:
//      subtype the new renderer subtype
//
// Programmer:  Kevin Griffin
// Creation:    Fri Mar 11 12:27:45 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
AnariRenderingWidget::rendererSubtypeChanged(const QString &subtype)
{
    auto rendererSubtype = subtype.toStdString();
    renderingAttributes->SetAnariRendererSubtype(rendererSubtype);

    auto libname = libraryName->text().trimmed().toStdString();
    auto libSubtype = librarySubtypes->currentText().toStdString();

    auto anariLibrary = anari::loadLibrary(libname.c_str(), anari_visit::StatusCallback);
    auto anariDevice = anari::newDevice(anariLibrary, libSubtype.c_str());

    if(anariDevice)
    {
        // Create Dynamic Widget
        auto key = libname + ":" + libSubtype + ":" + rendererSubtype;
        CreateDynamicWidget(anariDevice, rendererSubtype.c_str(), key, GetBackendType(libname) == BackendType::USD);

        // Clean-up
        anari::release(anariDevice, anariDevice);
        anariUnloadLibrary(anariLibrary);

        renderingWindow->SetUpdateApply(false);
    }
    else
    {
        debug1 << "Could not create the ANARI back-end device (" << libname << ") to update the Rendering UI." << std::endl;
        emit currentBackendChanged(0);
        renderingWindow->SetUpdateApply(false);
    }
}

void
AnariRenderingWidget::UpdateRenderingAttributes()
{
    auto widget = dynamicLayouts->currentWidget();
    auto children = widget->findChildren<QWidget *>();
    stringVector params;

    for(auto child : children)
    {
        auto name = child->objectName().toStdString();
        std::cout << "Renderer Parameter Name: " << name.c_str() << std::endl;

        if(qobject_cast<QSpinBox *>(child) != nullptr)
        {
            auto spinBox = qobject_cast<QSpinBox *>(child);
            auto val = spinBox->value();
            std::string valStr = name + ":" + std::to_string(val);
            params.push_back(valStr);
        }
        // else if(qobject_cast<QLineEdit *>(child) != nullptr)
        // {
        //     auto lineEdit = qobject_cast<QLineEdit *>(child);
        //     auto val = lineEdit->text().toFloat();
        //     renderingAttributes->SetAnariFloat32(name, val);
        // }
        // else if(qobject_cast<QCheckBox *>(child) != nullptr)
        // {
        //     auto checkBox = qobject_cast<QCheckBox *>(child);
        //     auto val = checkBox->isChecked();
        //     renderingAttributes->SetAnariBool(name, val);
        // }
        // else if(qobject_cast<QComboBox *>(child) != nullptr)
        // {
        //     auto comboBox = qobject_cast<QComboBox *>(child);
        //     auto val = comboBox->currentText().toStdString();
        //     renderingAttributes->SetAnariString(name, val);
        // }
    }
    // TODO: Update the rendering attributes with the new parameters
    // renderingAttributes->SetAnariRendererParams(params);
    renderingWindow->SetUpdateApply(false);
}

void AnariRenderingWidget::spinBoxValueChanged(int value)
{
    UpdateRenderingAttributes();
    renderingWindow->SetUpdateApply(false);
}

void
AnariRenderingWidget::lineEditingFinished()
{
    UpdateRenderingAttributes();
    renderingWindow->SetUpdateApply(false);
}

void
AnariRenderingWidget::comboBoxTextChanged(const QString &text)
{
    UpdateRenderingAttributes();
    renderingWindow->SetUpdateApply(false);
}

void
AnariRenderingWidget::checkBoxToggled(bool checked)
{
    UpdateRenderingAttributes();
    renderingWindow->SetUpdateApply(false);
}