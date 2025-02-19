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
#ifndef __SOLVER_VARIABLE_ECONOMY_ALL_H__
#define __SOLVER_VARIABLE_ECONOMY_ALL_H__

#include "../variable.h"
#include "../area.h"
#include "../setofareas.h"
#include "../bindConstraints.h"

#include "price.h"
#include "balance.h"
#include "../commons/load.h"
#include "../commons/wind.h"
#include "../commons/hydro.h"
#include "../commons/rowBalance.h"
#include "../commons/psp.h"
#include "../commons/miscGenMinusRowPSP.h"
#include "../commons/solar.h"
#include "../commons/join.h"
#include "../commons/spatial-aggregate.h"

// For General values
#include "dispatchableGeneration.h"
#include "thermalAirPollutantEmissions.h"
#include "renewableGeneration.h"
#include "overallCost.h"
#include "operatingCost.h"
#include "nonProportionalCost.h"
#include "nbOfDispatchedUnits.h"
#include "hydrostorage.h"
#include "pumping.h"
#include "reservoirlevel.h"
#include "inflow.h"
#include "overflow.h"
#include "waterValue.h"
#include "hydroCost.h"
#include "shortTermStorage.h"
#include "unsupliedEnergy.h"
#include "domesticUnsuppliedEnergy.h"
#include "localMatchingRuleViolations.h"
#include "spilledEnergyAfterCSR.h"
#include "dtgMarginAfterCsr.h"
#include "spilledEnergy.h"

#include "lold.h"
#include "lolp.h"
#include "max-mrg.h"

#include "avail-dispatchable-generation.h"
#include "dispatchable-generation-margin.h"

// By thermal plant
#include "productionByDispatchablePlant.h"
#include "npCostByDispatchablePlant.h"
#include "nbOfDispatchedUnitsByPlant.h"
#include "profitByPlant.h"

// By RES plant
#include "productionByRenewablePlant.h"

// Short term storage output variables by cluster
#include "STStorageInjectionByCluster.h"
#include "STStorageWithdrawalByCluster.h"
#include "STStorageLevelsByCluster.h"
#include "STStorageCashFlowByCluster.h"

// Output variables associated to links
#include "links/flowLinear.h"
#include "links/flowLinearAbs.h"
#include "links/loopFlow.h"
#include "links/flowQuad.h"
#include "links/hurdleCosts.h"
#include "links/congestionFee.h"
#include "links/congestionFeeAbs.h"
#include "links/marginalCost.h"
#include "links/congestionProbability.h"

// Output variables associated to binding constraints
#include "bindingConstraints/bindingConstraintsMarginalCost.h"

namespace Antares::Solver::Variable::Economy
{
/*!
** \brief All variables for a single link (economy)
*/
typedef FlowLinear             // Flow linear
  <FlowLinearAbs               // Flow linear Abs
   <LoopFlow                   // Loop flow
    <FlowQuad                  // Flow Quad
     <CongestionFee            // Congestion Fee
      <CongestionFeeAbs        // Congestion Fee (Abs)
       <MarginalCost           // Marginal Cost
        <CongestionProbability // Congestion Probability (+/-)
         <HurdleCosts          // Hurdle costs
          <>>>>>>>>>
    VariablePerLink;
// forward declaration
class Links;

/*!
** \brief All variables for a single area (economy)
*/
typedef                           // Prices
  OverallCost                     // Overall Cost (Op. Cost + Unsupplied Eng.)
  <OperatingCost                  // Operating Cost
   <Price                         // Marginal price
                                  // Thermal pollutants
    <ThermalAirPollutantEmissions // Overall pollutant emissions(from all thermal dispatchable
                                  // clusters) Production by thermal cluster
     <ProductionByDispatchablePlant    // Energy generated by thermal dispatchable clusters
      <ProductionByRenewablePlant      // Energy generated by renewable clusters (must-run)
       <Balance                        // Nodal Energy Balance
                                       // Misc Gen.
        <RowBalance                    // Misc Gen. Row balance
         <PSP                          // PSP
          <MiscGenMinusRowPSP          // Misc Gen. - Row Balance - PSP
                                       // Time series
           <TimeSeriesValuesLoad       // Load
            <TimeSeriesValuesHydro     // Hydro
             <TimeSeriesValuesWind     // Wind
              <TimeSeriesValuesSolar   // Solar
                                       // Other
               <DispatchableGeneration // All dispatchable generation
                <RenewableGeneration   // All renewable generation
                 <HydroStorage         // Hydro Storage Generation
                  <Pumping             // Pumping generation
                   <ReservoirLevel     // Reservoir levels
                    <Inflows           // Hydraulic inflows
                     <Overflows        // Hydraulic overflows
                      <WaterValue      // Water values
                       <HydroCost      // Hydro costs
                        <ShortTermStorageByGroup<STstorageInjectionByCluster<
                          STstorageWithdrawalByCluster<STstorageLevelsByCluster<
                          STstorageCashFlowByCluster<
                            UnsupliedEnergy           // Unsuplied Energy
                            <DomesticUnsuppliedEnergy // Domestic Unsupplied Energy
                             <LMRViolations           // LMR Violations
                              <SpilledEnergy          // Spilled Energy
                               <SpilledEnergyAfterCSR // SpilledEnergyAfterCSR
                                <LOLD                 // LOLD
                                 <LOLP                // LOLP
                                  <AvailableDispatchGen<DispatchableGenMargin<
                                    DtgMarginCsr // DTG MRG CSR
                                    <Marge<NonProportionalCost<
                                      NonProportionalCostByDispatchablePlant // Startup cost + Fixed
                                                                             // cost per thermal
                                                                             // plant detail
                                      <NbOfDispatchedUnits         // Number of Units Dispatched
                                       <NbOfDispatchedUnitsByPlant // Number of Units Dispatched by
                                                                   // plant
                                        <ProfitByPlant
                                         // Links
                                         <Variable::Economy::Links // All links
                                          >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    VariablesPerArea;

/*!
** \brief All variables for a single set of areas (economy)
*/
typedef // Prices
  Common::SpatialAggregate<
    OverallCost,
    Common::SpatialAggregate<
      OperatingCost,
      Common::SpatialAggregate<
        Price,
        // Thermal pollutants
        Common::SpatialAggregate<
          ThermalAirPollutantEmissions,
          // Production by thermal cluster
          Common::SpatialAggregate<
            Balance,
            // Misc Gen.
            Common::SpatialAggregate<
              RowBalance,
              Common::SpatialAggregate<
                PSP,
                Common::SpatialAggregate<
                  MiscGenMinusRowPSP,
                  // Time series
                  Common::SpatialAggregate<
                    TimeSeriesValuesLoad,
                    Common::SpatialAggregate<
                      TimeSeriesValuesHydro,
                      Common::SpatialAggregate<
                        TimeSeriesValuesWind,
                        Common::SpatialAggregate<
                          TimeSeriesValuesSolar,
                          // Other
                          Common::SpatialAggregate<
                            DispatchableGeneration,
                            Common::SpatialAggregate<
                              RenewableGeneration,
                              Common::SpatialAggregate<
                                HydroStorage,
                                Common::SpatialAggregate<
                                  Pumping,
                                  Common::SpatialAggregate<
                                    ReservoirLevel,
                                    Common::SpatialAggregate<
                                      Inflows,
                                      Common::SpatialAggregate<
                                        Overflows,
                                        Common::SpatialAggregate<
                                          WaterValue,
                                          Common::SpatialAggregate<
                                            HydroCost,
                                            Common::SpatialAggregate<
                                              ShortTermStorageByGroup,
                                              Common::SpatialAggregate<
                                                UnsupliedEnergy,
                                                Common::SpatialAggregate<
                                                  DomesticUnsuppliedEnergy,
                                                  Common::SpatialAggregate<
                                                    LMRViolations,
                                                    Common::SpatialAggregate<
                                                      SpilledEnergy,
                                                      Common::SpatialAggregate<
                                                        SpilledEnergyAfterCSR,
                                                        // LOLD
                                                        Common::SpatialAggregate<
                                                          LOLD,
                                                          Common::SpatialAggregate<
                                                            LOLP,
                                                            Common::SpatialAggregate<
                                                              AvailableDispatchGen,
                                                              Common::SpatialAggregate<
                                                                DispatchableGenMargin,
                                                                Common::SpatialAggregate<
                                                                  DtgMarginCsr,
                                                                  Common::SpatialAggregate<
                                                                    Marge,

                                                                    // Detail Prices
                                                                    Common::SpatialAggregate<
                                                                      NonProportionalCost, // MBO
                                                                                           // 13/05/2014
                                                                                           // -
                                                                                           // refs:
                                                                                           // #21

                                                                      // Number Of Dispatched Units
                                                                      Common::SpatialAggregate<
                                                                        NbOfDispatchedUnits // MBO
                                                                                            // 25/02/2016
                                                                                            // -
                                                                                            // refs:
                                                                                            // #55
                                                                        >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    VariablesPerSetOfAreas;

typedef BindingConstMarginCost< // Marginal cost for a binding constraint
  Container::EndOfList          // End of variable list
  >

  VariablesPerBindingConstraints;

typedef Variable::Join<
  // Variables for each area / links attached to the areas
  Variable::Areas<VariablesPerArea>,
  // Variables for each set of areas
  Variable::SetsOfAreas<VariablesPerSetOfAreas>,
  // Variables for each binding constraint
  Variable::BindingConstraints<VariablesPerBindingConstraints>>
  ItemList;

/*!
** \brief All variables for a simulation (economy)
*/
typedef Container::List<ItemList> AllVariables;

} // namespace Antares::Solver::Variable::Economy

// post include
#include "links.h"

#endif // __SOLVER_VARIABLE_ECONOMY_ALL_H__
