#include "adq_patch_post_process_list.h"
#include "../post_process_commands.h"


namespace Antares::Solver::Simulation
{

AdqPatchPostProcessList::AdqPatchPostProcessList(const AdqPatchParams& adqPatchParams,
                                                 PROBLEME_HEBDO* problemeHebdo,
                                                 uint thread_number,
                                                 AreaList& areas,
                                                 SheddingPolicy sheddingPolicy,
                                                 SimplexOptimization splxOptimization,
                                                 Calendar& calendar) 
    : interfacePostProcessList(problemeHebdo, thread_number)
{
    post_process_list.push_back(std::make_unique<DispatchableMarginPostProcessCmd>(
        problemeHebdo_, 
        thread_number_, 
        areas));
    // Here a post process particular to adq patch
    post_process_list.push_back(std::make_unique<CurtailmentSharingPostProcessCmd>(
        adqPatchParams,
        problemeHebdo_,
        areas,
        thread_number_));
    post_process_list.push_back(std::make_unique<HydroLevelsUpdatePostProcessCmd>(
        problemeHebdo_, 
        areas, 
        false, 
        false));
    post_process_list.push_back(std::make_unique<RemixHydroPostProcessCmd>(
        problemeHebdo_,
        areas,
        sheddingPolicy,
        splxOptimization,
        thread_number));
    // Here a post process particular to adq patch
    post_process_list.push_back(std::make_unique<DTGmarginForAdqPatchPostProcessCmd>(
        problemeHebdo_,
        areas,
        thread_number));
    post_process_list.push_back(std::make_unique<HydroLevelsUpdatePostProcessCmd>(
        problemeHebdo_,
        areas,
        true,
        false));
    post_process_list.push_back(std::make_unique<InterpolateWaterValuePostProcessCmd>(
        problemeHebdo_,
        areas,
        calendar));
    post_process_list.push_back(std::make_unique<HydroLevelsFinalUpdatePostProcessCmd>(
        problemeHebdo_,
        areas));
}

} // namespace Antares::Solver::Simulation