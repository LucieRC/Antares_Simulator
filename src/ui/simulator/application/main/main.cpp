/*
** Copyright 2007-2023 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/

#include "main.h"

#include <antares/study/study.h>

#include "../menus.h"
#include "../study.h"

#include <antares/date/date.h>

// Map
#include "../../toolbox/components/map/component.h"

// Datagrid
#include "../../toolbox/components/datagrid/component.h"
#include "../../toolbox/components/datagrid/gridhelper.h"

// MainPanel
#include "../../toolbox/components/mainpanel.h"

// Jobs
#include "../../toolbox/jobs.h"
#include <antares/study/scenario-builder/sets.h>
#include <ui/common/dispatcher/settings.h>
#include <ui/common/lock.h>
#include <ui/common/component/spotlight/spotlight.h>
#include <ui/common/component/frame/registry.h>

// Windows
#include "../../windows/inspector.h"
#include "../../windows/message.h"
#include "../../windows/bindingconstraint/bindingconstraint.h"

#include <wx/sizer.h>
#include <wx/utils.h>
#include <wx/wupdlock.h>

#include "internal-ids.h"
#include "internal-data.h"
#include "../wait.h"

#include "../../windows/options/advanced/advanced.h"

using namespace Yuni;

namespace Antares
{
namespace Forms
{
/*extern*/ String StudyToLoadAtStartup;

//! Global instance for the main form
static ApplWnd* gInstance = nullptr;

BEGIN_EVENT_TABLE(ApplWnd, wxFrame)

// Queue Service
EVT_MENU(mnIDExecuteQueueEvent, ApplWnd::evtOnExecuteQueueEvent)

// File
EVT_MENU(mnIDWizard, ApplWnd::evtOnWizard)
EVT_MENU(mnIDNew, ApplWnd::evtOnNewStudy)
EVT_MENU(mnIDOpen, ApplWnd::evtOnOpenStudy)
EVT_AUITOOLBAR_TOOL_DROPDOWN(mnIDOpen, ApplWnd::evtOnDropDownRecents)
EVT_MENU(mnIDClose, ApplWnd::evtOnCloseStudy)
EVT_MENU(mnIDCleanOpenedStudyFolder, ApplWnd::evtOnCleanCurrentStudyFolder)
EVT_MENU(mnIDOpenExplorer, ApplWnd::evtOnOpenStudyFolderInExplorer)
EVT_MENU(mnUpdateInterfaceAfterLoadingAStudy, ApplWnd::evtOnUpdateInterfaceAfterLoadedStudy)

// Recent files
EVT_MENU(mnIDOpenRecents_0, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_1, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_2, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_3, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_4, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_5, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_6, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_7, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_8, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDOpenRecents_9, ApplWnd::evtOnQuickOpenStudy)
EVT_MENU(mnIDQuit, ApplWnd::evtOnQuit)
EVT_MENU(mnIDSave, ApplWnd::evtOnSave)
EVT_MENU(mnIDSaveAs, ApplWnd::evtOnSaveAs)
EVT_MENU(mnIDExportMap, ApplWnd::evtOnExportMap)
EVT_MENU(mnIDOpenRecents_ToggleShowPaths, ApplWnd::evtOnOpenRecentsToggleShowPaths)
EVT_MENU(mnIDOpenRecents_ClearHistory, ApplWnd::evtOnOpenRecentsClearHistory)

// View
EVT_MENU(mnIDViewSystem, ApplWnd::evtOnViewSystem)
EVT_MENU(mnIDViewAllSystem, ApplWnd::evtOnViewAllSystem)
EVT_MENU(mnIDViewSimulation, ApplWnd::evtOnViewSimulation)
EVT_MENU(mnIDViewNotes, ApplWnd::evtOnViewNotes)
EVT_MENU(mnIDViewLoad, ApplWnd::evtOnViewLoad)
EVT_MENU(mnIDViewSolar, ApplWnd::evtOnViewSolar)
EVT_MENU(mnIDViewWind, ApplWnd::evtOnViewWind)
EVT_MENU(mnIDViewRenewable, ApplWnd::evtOnViewRenewable)
EVT_MENU(mnIDViewHydro, ApplWnd::evtOnViewHydro)
EVT_MENU(mnIDViewThermal, ApplWnd::evtOnViewThermal)
EVT_MENU(mnIDViewMiscGen, ApplWnd::evtOnViewMiscGen)
EVT_MENU(mnIDViewReservesDSM, ApplWnd::evtOnViewReservesDSM)
EVT_MENU(mnIDViewInterconnections, ApplWnd::evtOnViewInterconnections)
EVT_MENU(mnIDViewBindingConstraints, ApplWnd::evtOnViewBindingConstraints)
EVT_MENU(mnIDViewNodalOptimization, ApplWnd::evtOnViewNodalOptimization)
// -
EVT_MENU(mnIDStudyLogs, ApplWnd::evtOnStudyLogs)
EVT_MENU(mnIDMemoryUsedByTheStudy, ApplWnd::evtOnMemoryUsedByTheStudy)

// Study
EVT_MENU(mnIDStudyEditTitle, ApplWnd::evtOnSetStudyInfos)
EVT_MENU(mnIDStudyEditAuthors, ApplWnd::evtOnSetStudyInfos)
EVT_MENU(mnIDStudyEditMapSelectAll, ApplWnd::evtOnEditMapSelectAll)
EVT_MENU(mnIDStudyEditMapUnselectAll, ApplWnd::evtOnEditMapUnselectAll)
EVT_MENU(mnIDStudyEditMapReverseSelection, ApplWnd::evtOnEditMapReverseSelection)
EVT_MENU(mnIDStudyEditCopy, ApplWnd::evtOnEditCopy)
EVT_MENU(mnIDStudyEditPaste, ApplWnd::evtOnEditPaste)
EVT_MENU(mnIDStudyEditPasteSpecial, ApplWnd::evtOnEditPasteSpecial)
EVT_MENU(mnIDStudySessions, ApplWnd::evtOnStudySessions)
// Simulation
EVT_MENU(mnIDRunTheSimulation, ApplWnd::evtOnRunSimulation)
EVT_MENU(mnIDRunTheTSGenerators, ApplWnd::evtOnRunTSGenerators)
EVT_MENU(mnIDRunTheTSAnalyzer, ApplWnd::evtOnRunTSAnalyzer)
EVT_MENU(mnIDRunTheConstraintsBuilder, ApplWnd::evtOnRunConstraintsBuilder)
// Tools
EVT_MENU(mnIDCleanAStudyFolder, ApplWnd::evtOnCleanStudyFolder)

// Options
EVT_MENU(mnIDOptionTempFolder, ApplWnd::evtOnOptionsTempFolder)
EVT_MENU(mnIDOptionConfigureThematicTrimming, ApplWnd::evtOnOptionsSelectOutput)
EVT_MENU(mnIDOptionConfigureAreasTrimming, ApplWnd::evtOnOptionsSelectAreasTrimming)
EVT_MENU(mnIDOptionConfigureLinksTrimming, ApplWnd::evtOnOptionsSelectLinksTrimming)
EVT_MENU(mnIDOptionConfigureDistricts, ApplWnd::evtOnOptionsDistricts)
EVT_MENU(mnIDOptionConfigureMCScenarioPlaylist, ApplWnd::evtOnOptionsMCPlaylist)
EVT_MENU(mnIDOptionConfigureMCScenarioBuilder, ApplWnd::evtOnOptionsMCScenarioBuilder)
EVT_MENU(mnIDOptionOptimizationPrefs, ApplWnd::evtOnOptionsOptimizationPrefs)
EVT_MENU(mnIDOptionAdvanced, ApplWnd::evtOnOptionsAdvanced)
EVT_MENU(mnIDOptionAdequacyPatchOptions, ApplWnd::evtOnOptionsAdequacyPatchOptions)
EVT_MENU(mnIDOptionAdequacyPatchAreas, ApplWnd::evtOnOptionsAdequacyPatchAreas)

// Window
EVT_MENU(mnIDFullscreen, ApplWnd::evtOnFullscreen)
EVT_MENU(mnIDInspector, ApplWnd::evtOnInspector)
EVT_MENU(mnIDWindowRaise_0, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_1, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_2, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_3, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_4, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_5, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_6, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_7, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_8, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_9, ApplWnd::evtOnRaiseWindow)
EVT_MENU(mnIDWindowRaise_Other, ApplWnd::evtOnRaiseWindow)

// Help
EVT_MENU(mnIDHelpAbout, ApplWnd::evtOnHelpAbout)
EVT_MENU(mnIDHelpPDFSystemMapEditorReferenceGuide,
         ApplWnd::evtOnHelpPDFSystemMapEditorReferenceGuide)
EVT_MENU(mnIDHelpPDFExamplesLibrary, ApplWnd::evtOnHelpPDFExamplesLibrary)
EVT_MENU(mnIDHelpOnlineDocumentation, ApplWnd::evtOnHelpOnlineDocumentation)
EVT_MENU(mnInternalLogMessage, ApplWnd::onLogMessage)
EVT_MENU(mnIDLaunchAnalyzer, ApplWnd::evtLaunchAnalyzer)
EVT_MENU(mnIDLaunchConstraintsBuilder, ApplWnd::evtLaunchConstraintsBuilder)

EVT_MENU_OPEN(ApplWnd::evtOnMenuOpen)
EVT_MENU_CLOSE(ApplWnd::evtOnMenuClose)

// Misc
EVT_MOTION(ApplWnd::onMouseMoved)
EVT_CLOSE(ApplWnd::evtOnFrameClose)

END_EVENT_TABLE()

ApplWnd* ApplWnd::Instance()
{
    return gInstance;
}

ApplWnd::ApplWnd() :
 Component::Frame::WxLocalFrame(nullptr,
                                wxID_ANY,
                                wxT("Antares"),
                                wxDefaultPosition,
                                wxSize(minimalWidth, minimalHeight)),
 pMainSizer(nullptr),
 pMenu(nullptr),
 pMenuFile(nullptr),
 pMenuFileRecents(nullptr),
 pMenuEdit(nullptr),
 pMenuView(nullptr),
 pMenuInput(nullptr),
 pMenuInputCreation(nullptr),
 pMenuInputLastSaved(nullptr),
 pMenuOutput(nullptr),
 pMenuSimulation(nullptr),
 pMenuOptions(nullptr),
 pMenuGeographicTrimming(nullptr),
 pMenuAdequacyPatch(nullptr),
 pMenuWindow(nullptr),
 pMenuTools(nullptr),
 pMenuHelp(nullptr),
 pPopupMenuOperatorsGrid(nullptr),
 pMainMap(nullptr),
 pNotebook(nullptr),
 pSectionNotebook(nullptr),
 pMainPanel(nullptr),
 pageLoadTimeSeries(nullptr),
 pageLoadPrepro(nullptr),
 pageLoadCorrelation(nullptr),
 pageSolarTimeSeries(nullptr),
 pageSolarPrepro(nullptr),
 pageSolarCorrelation(nullptr),
 pageHydroTimeSeries(nullptr),
 pageHydroPrepro(nullptr),
 pageHydroCorrelation(nullptr),
 pageHydroAllocation(nullptr),
 pageWindTimeSeries(nullptr),
 pageWindPrepro(nullptr),
 pageWindPreproDailyProfile(nullptr),
 pageWindCorrelation(nullptr),
 pageThermalClusterList(nullptr),
 pageThermalTimeSeries(nullptr),
 pageThermalTimeSeriesFuelCost(nullptr),
 pageThermalTimeSeriesCO2Cost(nullptr),
 pageThermalPrepro(nullptr),
 pageThermalCommon(nullptr),
 pageRenewableClusterList(nullptr),
 pageRenewableCommon(nullptr),
 pageNodalOptim(nullptr),
 pWndBindingConstraints(nullptr),
 pMapContextMenu(nullptr),
 pUserNotes(nullptr),
 pMainNotebookAlreadyHasItsComponents(false),
 pLogFlusherTimer(nullptr),
 pWndLogs(nullptr),
 pGuiReady(false),
 pUpdateCountLocker(0),
 pCurrentEquipmentPage(0)
{
    // Setting the global instance
    gInstance = this;
    // Registering this windows to the internal event service
    Antares::Dispatcher::Settings::WindowForPendingDispatchers = this;
    Antares::Dispatcher::Settings::IDForWxDispatcher = mnIDExecuteQueueEvent;

    // Informations about the application
    wxTheApp->SetVendorName(wxT("RTE"));
    wxTheApp->SetAppName(wxT("Antares"));

    // Automatically close all windows if the study is closed
    OnStudyClosed.connect(&Component::Frame::Registry::CloseAllLocal);

    // Initialize all needed components
    internalInitialize();
}

ApplWnd::~ApplWnd()
{
    // Close all windows as soon as possible
    Component::Frame::Registry::CloseAllLocal();

    // This windows is no longer available
    Antares::Dispatcher::Settings::WindowForPendingDispatchers = nullptr;

    // Avoid SegV on Windows.
    SetFocus();

    logs.debug() << "destroying the main frame";
    // Display the hourglass when quitting
    showWIP();

    // remove all temporary objects
    purgeAllTemporaryObjects();

    // Stopping the action service
    Dispatcher::Stop();

    // Remove the inspector
    Window::Inspector::Destroy();

    // We are about to leave !
    onApplicationQuit();

    // Close the study, if not already done
    // see evtOnFrameClose()
    CloseTheStudy(false);

    // Cleanup
    OnStudyClosed.clear();
    OnStudyLoaded.clear();
    OnStudyAreasChanged.clear();
    OnStudyAreaDelete.clear();

    // Disconnect all events
    destroyBoundEvents();
    // Unregister the global pointer to the instance
    gInstance = nullptr;

    // Destroy the log viewer if not already done
    destroyLogsViewer();
    // Destroy logs
    destroyLogs();

    // Force the deletion of all children
    // DestroyChildren();

    // Remove the status bar
    SetStatusBar(nullptr);

    pAUIManager.UnInit();

    Data::StudyIconFile.clear();
    Data::StudyIconFile.shrink();

    delete pData;
    pData = nullptr;

    // Because components may access to this class, we should
    // destroy them here to avoid a corrupt vtable.
    if (GetSizer())
        GetSizer()->Clear(true);
}

void ApplWnd::selectSystem()
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    if (pNotebook)
        pNotebook->select(wxT("sys"), true);
}

static inline void EnableItem(wxMenuBar* menu, int id, bool opened)
{
    auto* item = menu->FindItem(id);
    if (item)
        item->Enable(opened);
}

void ApplWnd::evtOnUpdateGUIAfterStudyIO(bool opened)
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    // Get the study, for any purpose
    auto study = GetCurrentStudy();

    // No UI controls
    if (not pBigDaddy)
        return;

    // Close any opened windows
    Component::Spotlight::FrameClose();

    bool aboutToQuit = IsGUIAboutToQuit();
    if (aboutToQuit)
    {
        opened = false;

        // Closing all window logs
        // Destroy the log viewer if not already done
        destroyLogsViewer();
    }

    GUILocker locker;

    // remove all temporary objects
    purgeAllTemporaryObjects();

    // Update logs
    refreshStudyLogs();

    // Detach the current study
    if (pMainMap)
    {
        pMainMap->detachStudy();
        pMainMap->clear();
    }

    auto* menu = GetMenuBar();
    if (not menu or not pMainMap) // no component ?
    {
        resetDefaultStatusBarText();
        return;
    }

    wxWindowUpdateLocker updater(this);
    WIP::Locker wip;

    // Refresh the calendar
    StudyRefreshCalendar();

    if (pGuiReady)
    {
        EnableItem(menu, mnIDSave, opened);
        EnableItem(menu, mnIDSaveAs, opened);
        EnableItem(menu, mnIDExportMap, opened);
        EnableItem(menu, mnIDClose, opened);
        EnableItem(menu, mnIDOpenExplorer, opened);
        EnableItem(menu, mnIDCleanOpenedStudyFolder, opened);

        EnableItem(menu, mnIDStudySessions, opened);
        EnableItem(menu, mnIDStudyEditTitle, opened);
        EnableItem(menu, mnIDStudyEditAuthors, opened);

        // Edit
        EnableItem(menu, mnIDStudyEditCopy, opened);
        EnableItem(menu, mnIDStudyEditPaste, opened);
        EnableItem(menu, mnIDStudyEditPasteSpecial, opened);
        EnableItem(menu, mnIDStudyEditMapReverseSelection, opened);
        EnableItem(menu, mnIDStudyEditMapUnselectAll, opened);
        EnableItem(menu, mnIDStudyEditMapSelectAll, opened);

        // View
        EnableItem(menu, mnIDViewSystem, opened);
        EnableItem(menu, mnIDViewAllSystem, opened);
        EnableItem(menu, mnIDViewSimulation, opened);
        EnableItem(menu, mnIDViewNotes, opened);
        EnableItem(menu, mnIDViewLoad, opened);

        EnableItem(
          menu, mnIDViewSolar, opened && study->parameters.renewableGeneration.isAggregated());
        EnableItem(
          menu, mnIDViewWind, opened && study->parameters.renewableGeneration.isAggregated());
        EnableItem(
          menu, mnIDViewRenewable, opened && study->parameters.renewableGeneration.isClusters());

        EnableItem(menu, mnIDViewHydro, opened);
        EnableItem(menu, mnIDViewThermal, opened);
        EnableItem(menu, mnIDViewMiscGen, opened);
        EnableItem(menu, mnIDViewReservesDSM, opened);
        EnableItem(menu, mnIDViewInterconnections, opened);
        EnableItem(menu, mnIDViewBindingConstraints, opened);
        EnableItem(menu, mnIDViewNodalOptimization, opened);

        // Simulation
        EnableItem(menu, mnIDRunTheSimulation, opened);
        EnableItem(menu, mnIDRunTheTSGenerators, opened);
        EnableItem(menu, mnIDRunTheTSAnalyzer, opened);
        EnableItem(menu, mnIDRunTheConstraintsBuilder, opened);

        // Options
        EnableItem(menu, mnIDOptionConfigureMCScenarioBuilder, opened);
        EnableItem(menu, mnIDOptionConfigureMCScenarioPlaylist, opened);
        EnableItem(menu, mnIDOptionOptimizationPrefs, opened);
        EnableItem(menu, mnIDOptionAdvanced, opened);
        EnableItem(menu, mnIDOptionAdequacyPatchOptions, opened);
        EnableItem(menu, mnIDOptionAdequacyPatchAreas, opened);

        EnableItem(menu, mnIDInspector, opened);

        RefreshListOfOutputsForTheCurrentStudy();
        refreshMenuInput();
        refreshMenuOptions(study);
    }

    // Loading data from the current study, if any
    if (not aboutToQuit and !(!study))
    {
        // Attach the study to the map
        pMainMap->attachStudy(study);
        // Load data from the study
        pMainMap->loadFromAttachedStudy();
        // center the view
        pMainMap->recenterView();
    }

    // Updating the Caption
    pMainPanel->refreshFromStudy();

    // Keep informed all other dependencies that something has changed
    OnStudyAreasChanged();
    OnStudySettingsChanged();
    Window::Options::OnRenewableGenerationModellingChanged(true);

    // Make some components visible
    pAUIManager.GetPane(pBigDaddy).Show(opened);
    pAUIManager.Update();

    // Reset the status bar
    resetDefaultStatusBarText();

    // reload the user notes and districts
    if (not aboutToQuit and study)
    {
        loadUserNotes();
        loadSets();
    }

    // Force the focus to the main form
    // This action is mandatory to have the shortcuts of the map
    // work properly.
    // This method will refresh the form as well
    if (not aboutToQuit)
        delayForceFocus();

    // Refresh
    GetSizer()->Layout();
    Refresh();

    if (aboutToQuit)
    {
        if (GetSizer())
        {
            GetSizer()->Clear(true);
            pUserNotes = nullptr;
            pBigDaddy = nullptr;
            pMainSizer = nullptr;
            pData->wipPanel = nullptr;
        }
        Dispatcher::GUI::Close(this);
    }
}

void ApplWnd::requestUpdateGUIAfterStudyIO(const bool studyOpened)
{
    Yuni::Bind<void()> callback;
    callback.bind(this, &ApplWnd::evtOnUpdateGUIAfterStudyIO, studyOpened);
    Dispatcher::GUI::Post(callback, 50 /*ms*/);
}

void ApplWnd::purgeAllTemporaryObjects()
{
    delete pMapContextMenu;
    pMapContextMenu = nullptr;

    delete pPopupMenuOperatorsGrid;
    pPopupMenuOperatorsGrid = nullptr;
}

bool ApplWnd::excludeFromMenu()
{
    return true;
}

void ApplWnd::updateOpenWindowsMenu()
{
    createMenuWindow();
}

void ApplWnd::saveStudy()
{
    if (CurrentStudyIsValid())
        Antares::SaveStudy();
}

void ApplWnd::saveStudyAs(const String& path, bool copyoutput, bool copyuserdata, bool copylogs)
{
    if (CurrentStudyIsValid())
        Antares::SaveStudyAs(path, copyoutput, copyuserdata, copylogs);
}

void ApplWnd::exportMap(const Yuni::String& path,
                        bool transparentBackground,
                        const wxColour& backgroundColor,
                        const std::list<uint16_t>& layers,
                        int nbSplitParts,
                        Antares::Map::mapImageFormat format)
{
    if (CurrentStudyIsValid())
        Antares::ExportMap(
          path, transparentBackground, backgroundColor, layers, nbSplitParts, format);
}

uint ApplWnd::mainNotebookCurrentEquipmentPage() const
{
    return pCurrentEquipmentPage;
}

void ApplWnd::onMainNotebookPageChanging(Component::Notebook::Page& page)
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    // Edit, current location
    pData->editCurrentLocation(page.caption());

    if (page.name() == wxT("load"))
        pCurrentEquipmentPage = Data::timeSeriesLoad;
    else if (page.name() == wxT("thermal"))
        pCurrentEquipmentPage = Data::timeSeriesThermal;
    else if (page.name() == wxT("renewable"))
        pCurrentEquipmentPage = Data::timeSeriesRenewable;
    else if (page.name() == wxT("solar"))
        pCurrentEquipmentPage = Data::timeSeriesSolar;
    else if (page.name() == wxT("wind"))
        pCurrentEquipmentPage = Data::timeSeriesWind;
    else if (page.name() == wxT("hydro"))
        pCurrentEquipmentPage = Data::timeSeriesHydro;
    else
        pCurrentEquipmentPage = 0;

    // Updating the menu EDIT
    {
        const bool pageIsMap = (page.name() == wxT("sys"));
        wxMenuBar* menu = GetMenuBar();

        EnableItem(menu, mnIDStudyEditCopy, pageIsMap);
        EnableItem(menu, mnIDStudyEditPaste, pageIsMap);
        EnableItem(menu, mnIDStudyEditPasteSpecial, pageIsMap);
        EnableItem(menu, mnIDStudyEditMapReverseSelection, pageIsMap);
        EnableItem(menu, mnIDStudyEditMapUnselectAll, pageIsMap);
        EnableItem(menu, mnIDStudyEditMapSelectAll, pageIsMap);
    }

    // Reset the status bar
    resetDefaultStatusBarText();

    // Keep informed all other dependencies that something may have changed
    if (0 and GUIFlagInvalidateAreas) // deprecated
    {
        OnStudyAreasChanged();
        GUIFlagInvalidateAreas = false;
    }
    if (SystemParameterHaveChanged)
    {
        OnStudySettingsChanged();
        SystemParameterHaveChanged = false;
    }

    if (page.name() == wxT("bindingconstraints"))
        OnStudyConstraintModified(nullptr);

    // Notify any subscriber that the selection of the main notebook
    // has changed.
    OnMainNotebookChanged();
}

class JobLoadScenarioBuilder final : public Toolbox::Jobs::Job
{
public:
    JobLoadScenarioBuilder(Data::Study& study) :
     Toolbox::Jobs::Job(wxT("Scenario Builder"), wxT("Loading data"), "images/32x32/open.png"),
     pStudy(study),
     pSets(nullptr)
    {
    }

    //! Destructor
    virtual ~JobLoadScenarioBuilder()
    {
    }

    //! Sets
    Data::ScenarioBuilder::Sets* scenarioBuilder() const
    {
        return pSets;
    }

protected:
    /*!
     * \brief Load a study from a folder
     */
    virtual bool executeTask() override
    {
        // Logs
        // logs.notice() << "Scenario Builder";

        pSets = new Data::ScenarioBuilder::Sets();
        pSets->loadFromStudy(pStudy);

        // The task is complete
        return true;
    }

private:
    //! Our study
    Data::Study& pStudy;
    //!
    Data::ScenarioBuilder::Sets* pSets;

}; // class JobLoadScenarioBuilder

void ApplWnd::onSectionNotebookPageChanging(Component::Notebook::Page& page)
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    if (pScenarioBuilderNotebook)
        pScenarioBuilderNotebook->select(wxT("load"), true);

    // Scenario Builder
    auto study = GetCurrentStudy();
    if (page.name() == wxT("scenariobuilder"))
    {
        if (!(!study) and not study->scenarioRules)
        {
            // Load in background the data, which may take some time
            auto* job = new JobLoadScenarioBuilder(*study);
            wxTheApp->Yield();
            job->run();
            study->scenarioRules = job->scenarioBuilder();
            job->Destroy();
        }

        // Notify components that the data are ready
        // Force the reload in the same time
        OnStudyScenarioBuilderDataAreLoaded();
        // required otherwise the matrix won't be regenerated
        page.refresh();
    }

    // Reset the status bar
    resetDefaultStatusBarText();
}

void ApplWnd::onSystemParametersChanged()
{
    // Do nothing
}

void ApplWnd::refreshHomePageOnRenewableModellingChanged(bool aggregated, bool init)
{
    // Main window
    for (auto s : {"wind", "solar"})
        pNotebook->set_page_visibility(wxString(s), aggregated);
    pNotebook->set_page_visibility(wxString("renewable"), not aggregated);

    // Page selection after the renewable modelling changed
    if (!init)
    {
        const Component::Notebook::Page* windPage = pNotebook->find("wind");
        const Component::Notebook::Page* solarPage = pNotebook->find("solar");
        const Component::Notebook::Page* renewablePage = pNotebook->find("renewable");
        if (aggregated)
        {
            if (pNotebook->selected() == renewablePage)
                pNotebook->select(wxT("wind"));
        }
        else
        {
            if (pNotebook->selected() == windPage || pNotebook->selected() == solarPage)
                pNotebook->select(wxT("renewable"));
        }
        pNotebook->forceRefresh();
    }
}

void ApplWnd::refreshScenarioBuilderPagOnRenewableModellingChanged(bool aggregated)
{
    for (auto s : {"wind", "solar"})
        pScenarioBuilderNotebook->set_page_visibility(wxString(s), aggregated);

    pScenarioBuilderNotebook->set_page_visibility(wxString("renewable"), not aggregated);
}

void ApplWnd::refreshInputMenuOnRenewableModellingChanged(bool aggregated)
{
    auto* menu = GetMenuBar();
    EnableItem(menu, mnIDViewSolar, aggregated);
    EnableItem(menu, mnIDViewWind, aggregated);
    EnableItem(menu, mnIDViewRenewable, not aggregated);
}

void ApplWnd::onRenewableGenerationModellingChanged(bool init)
{
    auto study = GetCurrentStudy();
    if (!study)
        return;

    const bool aggregated = study->parameters.renewableGeneration.isAggregated();

    refreshHomePageOnRenewableModellingChanged(aggregated, init);
    refreshScenarioBuilderPagOnRenewableModellingChanged(aggregated);
    refreshInputMenuOnRenewableModellingChanged(aggregated);
}

void ApplWnd::title()
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");
    wxString t;
    t << wxT("Antares Simulator")
#ifndef NDEBUG
      << wxT(" - DEVELOPER PREVIEW")
#endif
#if ANTARES_RC != 0
      << wxT(" - RELEASE CANDIDATE - rc") << int(ANTARES_RC)
#endif
      ;
    SetLabel(t);
    SetTitle(t);
    wxTheApp->SetAppName(t);
}

void ApplWnd::title(const wxString& s)
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");
    wxString t;
    if (StudyHasBeenModified())
        t << wxT("* ");
    t << (s.IsEmpty() ? wxT("<untitled>") : s) << wxT(" - Antares Simulator")
#ifndef NDEBUG
      << wxT(" - DEVELOPER PREVIEW")
#endif
#if ANTARES_RC != 0
      << wxT(" - RELEASE CANDIDATE - rc") << int(ANTARES_RC)
#endif
      ;
    SetLabel(t);
    SetTitle(t);
    wxTheApp->SetAppName(t);
}

bool ApplWnd::wouldYouLikeToSaveTheStudy()
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    auto study = GetCurrentStudy();
    if (!(!study))
    {
        if (StudyHasBeenModified())
        {
            wxString t;
            t << wxT("Would you like to save changes for the study \"")
              << wxStringFromUTF8(study->header.caption) << wxT("\" ?");

            Window::Message message(
              this, wxT("Save"), wxT("The study has been modified"), t, "images/misc/save.png");
            message.add(Window::Message::btnSaveChanges);
            if (IsGUIAboutToQuit())
                message.add(Window::Message::btnQuitWithoutSaving, false, 15);
            else
                message.add(Window::Message::btnDiscard, false, 15);
            message.add(Window::Message::btnCancel, true);

            switch (message.showModal())
            {
            case Window::Message::btnSaveChanges:
            {
                const SaveResult r = SaveStudy();
                return (r == svsSaved or r == svsDiscard);
            }
            case Window::Message::btnQuitWithoutSaving:
            case Window::Message::btnDiscard:
                return true;
            default:
                return false;
            }
        }
    }
    return true;
}

void ApplWnd::evtOnUpdateInterfaceAfterLoadedStudy(wxCommandEvent&)
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    auto studyptr = GetCurrentStudy();
    if (not studyptr)
    {
        requestUpdateGUIAfterStudyIO(false);
        return;
    }

    auto& study = *studyptr;
    // remove all temporary objects
    purgeAllTemporaryObjects();

    logs.info() << LOG_UI << "The study has changed: Updating the interface accordingly...";

    // Added it to the list of recent files
    if (not study.folder.empty())
    {
        Menu::AddRecentFile(menuRecentFiles(),
                            wxStringFromUTF8(study.header.caption),
                            wxStringFromUTF8(study.folder));
    }

    // User notes
    selectSystem();

    // Logs
    // Updating the GUI
    requestUpdateGUIAfterStudyIO(true);
}

void ApplWnd::onStudyLoaded()
{
    selectAllDefaultPages();
}

void ApplWnd::selectAllDefaultPages()
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    if (pageLoadTimeSeries)
        pageLoadTimeSeries->select();
    if (pageSolarTimeSeries)
        pageSolarTimeSeries->select();
    if (pageHydroTimeSeries)
        pageHydroTimeSeries->select();
    if (pageWindTimeSeries)
        pageWindTimeSeries->select();
    if (pageThermalClusterList)
        pageThermalClusterList->select();
    if (pageThermalCommon)
        pageThermalCommon->select();
    if (pageRenewableClusterList)
        pageRenewableClusterList->select();
    if (pageRenewableCommon)
        pageRenewableCommon->select();
    if (pageLinksParameters)
        pageLinksParameters->select();
    if (pageLinksNTC)
        pageLinksNTC->select();
    if (pageWindPreproDailyProfile)
        pageWindPreproDailyProfile->select();
    if (pWndBindingConstraints)
        pWndBindingConstraints->selectDefaultPage();
    if (pageNodalOptim)
        pageNodalOptim->select();
    if (pSectionNotebook)
        pSectionNotebook->select(wxT("input"));
    if (pScenarioBuilderNotebook)
        pScenarioBuilderNotebook->select(wxT("load"));
}

void ApplWnd::hideAllComponentsRelatedToTheStudy()
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");
    pAUIManager.GetPane(pBigDaddy).Hide();
    pAUIManager.Update();
}

void ApplWnd::backgroundTimerStop() const
{
    assert(wxIsMainThread() == true and "Must be ran from the main thread");

    // !! It is extremly important to wait for all jobs to finish
    // Otherwise, it may occur a race condition with another thread.
    // It would make possible to flush all variables while accessing them.
    Dispatcher::Wait();
}

void ApplWnd::evtOnMenuOpen(wxMenuEvent&)
{
    // do nothing
}

void ApplWnd::evtOnMenuClose(wxMenuEvent&)
{
    resetDefaultStatusBarText();
}

void ApplWnd::evtOnContextMenuMap(int x, int y)
{
    if (pMainMap)
    {
        // create the menu if not already exist
        if (not pMapContextMenu)
            pMapContextMenu = createMenuEdit();
        pMainMap->PopupMenu(pMapContextMenu, x, y);
    }
}

void ApplWnd::onMouseMoved(wxMouseEvent&)
{
    Component::Panel::OnMouseMoveFromExternalComponent();
}

bool ApplWnd::isScenarioBuilderOpened() const
{
    return pSectionNotebook->selected()
           and pSectionNotebook->selected()->name() == wxT("scenariobuilder");
}

void ApplWnd::backToInputData()
{
    if (pSectionNotebook)
        pSectionNotebook->select(wxT("input"), true);
}

void ApplWnd::evtOnExecuteQueueEvent(wxCommandEvent&)
{
    Antares::Dispatcher::Internal::ExecuteQueueDispatcher();
}

bool ApplWnd::isInWIPMode() const
{
    return (pData) and (pData->wipEnabled);
}

} // namespace Forms
} // namespace Antares
