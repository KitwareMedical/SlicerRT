/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// DoseVolumeHistogram includes
#include "vtkSlicerDoseVolumeHistogramModuleLogic.h"
#include "vtkSlicerDoseVolumeHistogramComparisonLogic.h"
#include "vtkMRMLDoseVolumeHistogramNode.h"

// SlicerRt includes
#include "vtkSlicerRtCommon.h"
#include "vtkPlanarContourToClosedSurfaceConversionRule.h"

// Segmentations includes
#include "vtkMRMLSegmentationNode.h"
#include "vtkSlicerSegmentationsModuleLogic.h"

// SegmentationCore includes
#include "vtkOrientedImageData.h"
#include "vtkSegmentationConverterFactory.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotSeriesNode.h>
#include <vtkMRMLPlotViewNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkImageAccumulate.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkTable.h>
#include <vtkTimerLog.h>

// ITK includes
#include "itkFactoryRegistration.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

std::string csvSeparatorCharacter(",");

//-----------------------------------------------------------------------------
int CompareCsvDvhTables(std::string dvhMetricsCsvFileName, std::string baselineCsvFileName,
                        double maxDose, double volumeDifferenceCriterion, double doseToAgreementCriterion,
                        double &agreementAcceptancePercentage);

int CompareCsvDvhMetrics(std::string dvhMetricsCsvFileName, std::string baselineDvhMetricCsvFileName, double metricDifferenceThreshold);

//-----------------------------------------------------------------------------
int vtkSlicerDoseVolumeHistogramModuleLogicTest1( int argc, char * argv[] )
{
  int argIndex = 1;

  // TestSceneFile
  const char *testSceneFileName  = nullptr;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-TestSceneFile") == 0)
    {
      testSceneFileName = argv[argIndex+1];
      std::cout << "Test MRML scene file name: " << testSceneFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      testSceneFileName = "";
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // BaselineDvhTableCsvFile
  const char *baselineDvhTableCsvFileName = nullptr;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-BaselineDvhTableCsvFile") == 0)
    {
      baselineDvhTableCsvFileName = argv[argIndex+1];
      std::cout << "Baseline DVH table CSV file name: " << baselineDvhTableCsvFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      baselineDvhTableCsvFileName = "";
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // BaselineDvhMetricCsvFile
  const char *baselineDvhMetricCsvFileName = nullptr;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-BaselineDvhMetricCsvFile") == 0)
    {
      baselineDvhMetricCsvFileName = argv[argIndex+1];
      std::cout << "Baseline DVH metric CSV file name: " << baselineDvhMetricCsvFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      baselineDvhMetricCsvFileName = "";
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // TemporarySceneFile
  const char *temporarySceneFileName = nullptr;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-TemporarySceneFile") == 0)
    {
      temporarySceneFileName = argv[argIndex+1];
      std::cout << "Temporary scene file name: " << temporarySceneFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      temporarySceneFileName = "";
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // TemporaryDvhTableCsvFile
  const char *temporaryDvhTableCsvFileName = nullptr;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-TemporaryDvhTableCsvFile") == 0)
    {
      temporaryDvhTableCsvFileName = argv[argIndex+1];
      std::cout << "Temporary DVH table CSV file name: " << temporaryDvhTableCsvFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      temporaryDvhTableCsvFileName = "";
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // TemporaryDvhMetricCsvFile
  const char *temporaryDvhMetricCsvFileName = nullptr;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-TemporaryDvhMetricCsvFile") == 0)
    {
      temporaryDvhMetricCsvFileName = argv[argIndex+1];
      std::cout << "Temporary DVH metric CSV file name: " << temporaryDvhMetricCsvFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      temporaryDvhMetricCsvFileName = "";
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // AutomaticOversamplingCalculation
  bool automaticOversamplingCalculation = false;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-AutomaticOversamplingCalculation") == 0)
    {
      automaticOversamplingCalculation = (vtkVariant(argv[argIndex+1]).ToInt() > 0 ? true : false);
      std::cout << "Automatic oversampling calculation: " << (automaticOversamplingCalculation ? "true" : "false") << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // VolumeDifferenceCriterion
  double volumeDifferenceCriterion = 0.0;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-VolumeDifferenceCriterion") == 0)
    {
      volumeDifferenceCriterion = vtkVariant(argv[argIndex+1]).ToDouble();
      std::cout << "Volume difference criterion: " << volumeDifferenceCriterion << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // DoseToAgreementCriterion
  double doseToAgreementCriterion = 0.0;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-DoseToAgreementCriterion") == 0)
    {
      doseToAgreementCriterion = vtkVariant(argv[argIndex+1]).ToDouble();
      std::cout << "Dose-to-agreement criterion: " << doseToAgreementCriterion << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // AgreementAcceptancePercentageThreshold
  double agreementAcceptancePercentageThreshold = 0.0;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-AgreementAcceptancePercentageThreshold") == 0)
    {
      agreementAcceptancePercentageThreshold = vtkVariant(argv[argIndex+1]).ToDouble();
      std::cout << "Agreement acceptance percentage threshold: " << agreementAcceptancePercentageThreshold << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // MetricDifferenceThreshold
  double metricDifferenceThreshold = 0.0;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-MetricDifferenceThreshold") == 0)
    {
      metricDifferenceThreshold = vtkVariant(argv[argIndex+1]).ToDouble();
      std::cout << "Metric difference threshold: " << metricDifferenceThreshold << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // DvhStartValue
  double dvhStartValue = 0.0;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-DvhStartValue") == 0)
    {
      dvhStartValue = vtkVariant(argv[argIndex+1]).ToDouble();
      std::cout << "DVH start value: " << dvhStartValue << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // DvhStepSize
  double dvhStepSize = 0.0;
  if (argc > argIndex+1)
  {
    if (STRCASECMP(argv[argIndex], "-DvhStepSize") == 0)
    {
      dvhStepSize = vtkVariant(argv[argIndex+1]).ToDouble();
      std::cout << "DVH step size: " << dvhStepSize << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // DoseSurfaceHistogram
  bool doseSurfaceHistogram = false;
  if (argc > argIndex + 1)
  {
    if (STRCASECMP(argv[argIndex], "-DoseSurfaceHistogram") == 0)
    {
      doseSurfaceHistogram = (vtkVariant(argv[argIndex + 1]).ToInt() > 0 ? true : false);
      std::cout << "Dose surface histogram: " << (doseSurfaceHistogram ? "true" : "false") << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }
  // UseInsideSurface
  bool useInsideSurface = false;
  if (argc > argIndex + 1)
  {
    if (STRCASECMP(argv[argIndex], "-UseInsideSurface") == 0)
    {
      useInsideSurface = (vtkVariant(argv[argIndex + 1]).ToInt() > 0 ? true : false);
      std::cout << "Dose surface histogram -- Inside surface: " << (useInsideSurface ? "true" : "false") << std::endl;
      argIndex += 2;
    }
  }
  else
  {
    std::cerr << "Invalid arguments" << std::endl;
    return EXIT_FAILURE;
  }

  // Constraint the criteria to be greater than zero
  if (volumeDifferenceCriterion == 0.0)
  {
    volumeDifferenceCriterion = EPSILON;
  }
  if (doseToAgreementCriterion == 0.0)
  {
    doseToAgreementCriterion = EPSILON;
  }
  if (metricDifferenceThreshold == 0.0)
  {
    metricDifferenceThreshold = EPSILON;
  }

  // Make sure NRRD reading works
  itk::itkFactoryRegistration();

  // Register planar contour to closed surface conversion rule
  vtkSegmentationConverterFactory::GetInstance()->RegisterConverterRule(
    vtkSmartPointer<vtkPlanarContourToClosedSurfaceConversionRule>::New() );

  // Create scene
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();

  // Create Segmentations logic
  vtkSmartPointer<vtkSlicerSegmentationsModuleLogic> segmentationsLogic = vtkSmartPointer<vtkSlicerSegmentationsModuleLogic>::New();
  segmentationsLogic->SetMRMLScene(mrmlScene);

  // Load test scene into temporary scene
  mrmlScene->SetURL(testSceneFileName);
  mrmlScene->Import();
  // Trigger resolving subject hierarchies after import (merging the imported one with the pseudo-singleton one).
  // Normally this is done by the plugin logic, but it is a Qt class, so we need to trigger it manually from a VTK-only environment.
  vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(mrmlScene);

  // Save it to the temporary directory
  vtksys::SystemTools::RemoveFile(temporarySceneFileName);
  mrmlScene->SetRootDirectory( vtksys::SystemTools::GetParentDirectory(temporarySceneFileName).c_str() );
  mrmlScene->SetURL(temporarySceneFileName);
  mrmlScene->Commit();

  // Get dose volume
  vtkSmartPointer<vtkCollection> doseVolumeNodes = vtkSmartPointer<vtkCollection>::New();
  std::vector<vtkMRMLNode*> volumeNodes;
  mrmlScene->GetNodesByClass("vtkMRMLScalarVolumeNode", volumeNodes);
  for (std::vector<vtkMRMLNode*>::iterator volumeNodeIt=volumeNodes.begin(); volumeNodeIt!=volumeNodes.end(); ++volumeNodeIt)
  {
    if (vtkSlicerRtCommon::IsDoseVolumeNode(*volumeNodeIt))
    {
      doseVolumeNodes->AddItem(*volumeNodeIt);
    }
  }
  if (doseVolumeNodes->GetNumberOfItems() != 1)
  {
    mrmlScene->Commit();
    std::cerr << "ERROR: Failed to get dose volume" << std::endl;
    return EXIT_FAILURE;
  }
  vtkMRMLScalarVolumeNode* doseScalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(doseVolumeNodes->GetItemAsObject(0));

  // Get segmentation node
  vtkSmartPointer<vtkCollection> segmentationNodes = vtkSmartPointer<vtkCollection>::Take(
    mrmlScene->GetNodesByClass("vtkMRMLSegmentationNode") );
  if (segmentationNodes->GetNumberOfItems() != 1)
  {
    mrmlScene->Commit();
    std::cerr << "ERROR: Failed to get segmentation" << std::endl;
    return EXIT_FAILURE;
  }
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(segmentationNodes->GetItemAsObject(0));

  // Determine maximum dose
  vtkNew<vtkImageAccumulate> doseStat;
  doseStat->SetInputData(doseScalarVolumeNode->GetImageData());
  doseStat->Update();
  double maxDose = doseStat->GetMax()[0];

  // Create and set up logic
  vtkSmartPointer<vtkSlicerDoseVolumeHistogramModuleLogic> dvhLogic = vtkSmartPointer<vtkSlicerDoseVolumeHistogramModuleLogic>::New();
  dvhLogic->SetMRMLScene(mrmlScene);

  // Create and set up parameter set MRML node
  vtkSmartPointer<vtkMRMLDoseVolumeHistogramNode> paramNode = vtkSmartPointer<vtkMRMLDoseVolumeHistogramNode>::New();
  mrmlScene->AddNode(paramNode);
  paramNode->SetAndObserveDoseVolumeNode(doseScalarVolumeNode);
  paramNode->SetAndObserveSegmentationNode(segmentationNode);
  paramNode->SetAutomaticOversampling(automaticOversamplingCalculation);
  paramNode->SetDoseSurfaceHistogram(doseSurfaceHistogram);
  paramNode->SetUseInsideDoseSurface(useInsideSurface);

  // Setup chart node
  vtkMRMLPlotChartNode* chartNode = paramNode->GetChartNode();
  if (!chartNode)
  {
    mrmlScene->Commit();
    std::cerr << "ERROR: Chart node must exist for DVH parameter set node" << std::endl;
    return EXIT_FAILURE;
  }

  // Set start value and step size if specified
  if (dvhStartValue != 0.0 && dvhStepSize != 0.0)
  {
    dvhLogic->SetStartValue(dvhStartValue);
    dvhLogic->SetStepSize(dvhStepSize);
  }

  // Setup time measurement
  vtkSmartPointer<vtkTimerLog> timer = vtkSmartPointer<vtkTimerLog>::New();
  double checkpointStart = timer->GetUniversalTime();
  UNUSED_VARIABLE(checkpointStart); // Although it is used later, a warning is logged so needs to be suppressed

  // Compute DVH
  std::string errorMessage = dvhLogic->ComputeDvh(paramNode);
  if (!errorMessage.empty())
  {
    std::cerr << errorMessage << std::endl;
    return EXIT_FAILURE;
  }

  // Calculate and print oversampling factors if automatically calculated
  if (automaticOversamplingCalculation)
  {
    std::map<std::string, double> oversamplingFactors;
    paramNode->GetAutomaticOversamplingFactors(oversamplingFactors);
    for (std::map<std::string, double>::iterator factorIt = oversamplingFactors.begin(); factorIt != oversamplingFactors.end(); ++factorIt)
    {
      std::cout << "  Automatic oversampling factor for segment " << factorIt->first << " calculated to be " << factorIt->second << std::endl;
    }
  }

  // Report time measurement
  double checkpointEnd = timer->GetUniversalTime();
  UNUSED_VARIABLE(checkpointEnd); // Although it is used just below, a warning is logged so needs to be suppressed
  std::cout << "DVH computation time (including rasterization): " << checkpointEnd-checkpointStart << " s" << std::endl;

  std::vector<vtkMRMLTableNode*> dvhNodes;
  paramNode->GetDvhTableNodes(dvhNodes);

  // Add DVH tables to chart node
  vtkNew<vtkMRMLPlotViewNode> plotViewNode;
  mrmlScene->AddNode(plotViewNode);
  std::vector<vtkMRMLTableNode*>::iterator dvhIt;
  for (dvhIt = dvhNodes.begin(); dvhIt != dvhNodes.end(); ++dvhIt)
  {
    if (!(*dvhIt))
    {
      std::cerr << "ERROR: Invalid DVH node" << std::endl;
      return EXIT_FAILURE;
    }

    vtkMRMLPlotSeriesNode* plotSeriesNode = dvhLogic->AddDvhToChart(chartNode, (*dvhIt));
    if (!plotSeriesNode)
    {
      std::cerr << "ERROR: Unable to add DVH to chart" << std::endl;
      return EXIT_FAILURE;
    }
  }

  mrmlScene->Commit();

  // Export DVH to CSV
  vtksys::SystemTools::RemoveFile(temporaryDvhTableCsvFileName);
  dvhLogic->ExportDvhToCsv(paramNode, temporaryDvhTableCsvFileName);

  // Compute DVH metrics
  paramNode->SetVDoseValues("5, 20");
  paramNode->SetShowVMetricsCc(true);
  paramNode->SetShowVMetricsPercent(true);
  dvhLogic->ComputeVMetrics(paramNode);

  paramNode->SetDVolumeValuesCc("2, 5");
  paramNode->SetDVolumeValuesPercent("5, 10");
  paramNode->SetShowDMetrics(true);
  dvhLogic->ComputeDMetrics(paramNode);

  vtksys::SystemTools::RemoveFile(temporaryDvhMetricCsvFileName);
  dvhLogic->ExportDvhMetricsToCsv(paramNode, temporaryDvhMetricCsvFileName);

  bool returnWithSuccess = true;

  // Compare CSV DVH tables
  double agreementAcceptancePercentage = -1.0;
  if (vtksys::SystemTools::FileExists(baselineDvhTableCsvFileName))
  {
    if (CompareCsvDvhTables(temporaryDvhTableCsvFileName, baselineDvhTableCsvFileName, maxDose,
      volumeDifferenceCriterion, doseToAgreementCriterion, agreementAcceptancePercentage) > 0)
    {
      std::cerr << "Failed to compare DVH table to baseline" << std::endl;
      returnWithSuccess = false;
    }
  }
  else
  {
    std::cerr << "Failed to open baseline DVH table: " << baselineDvhTableCsvFileName << std::endl;
    returnWithSuccess = false;
  }

  std::cout << "Agreement percentage: " << std::fixed << std::setprecision(2) << agreementAcceptancePercentage << "% (acceptance rate: " << agreementAcceptancePercentageThreshold << "%)" << std::endl;

  if (agreementAcceptancePercentage < agreementAcceptancePercentageThreshold)
  {
    std::cerr << "Agreement acceptance percentage is below threshold: " << std::fixed << std::setprecision(2) << agreementAcceptancePercentage
      << " < " << agreementAcceptancePercentageThreshold << std::endl;
    returnWithSuccess = false;
  }

  // Compare CSV DVH metrics
  if (vtksys::SystemTools::FileExists(baselineDvhMetricCsvFileName)) // TODO: add warning when all the metric tables can be compared
  {
    if (CompareCsvDvhMetrics(temporaryDvhMetricCsvFileName, baselineDvhMetricCsvFileName, metricDifferenceThreshold) > 0)
    {
      std::cerr << "Failed to compare DVH table to baseline" << std::endl;
      returnWithSuccess = false;
    }
    else
    {
      std::cout << "DVH metrics are within threshold compared to baseline." << std::endl;
    }
  }

  if (!returnWithSuccess)
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
// IMPORTANT: The baseline table has to be the one with smaller resolution!
int CompareCsvDvhTables(std::string dvhCsvFileName, std::string baselineCsvFileName,
                        double maxDose, double volumeDifferenceCriterion, double doseToAgreementCriterion,
                        double &agreementAcceptancePercentage)
{
  std::cout << "Comparing DVHs" << std::endl << "  Current: " << dvhCsvFileName << std::endl << "  Baseline: " << baselineCsvFileName << std::endl;

  if (!vtksys::SystemTools::FileExists(baselineCsvFileName.c_str()))
  {
    std::cerr << "Loading baseline CSV DVH table from file '" << baselineCsvFileName << "' failed - the file does not exist" << std::endl;
    return 1;
  }
  
  // Create and set up logic
  vtkSmartPointer<vtkSlicerDoseVolumeHistogramModuleLogic> csvReadLogic = vtkSmartPointer<vtkSlicerDoseVolumeHistogramModuleLogic>::New();

  // Collections of vtkDoubleArrays, with each vtkDoubleArray representing a structure and containing
  // an array of tuples which represent the dose and volume for the bins in that structure.
  vtkSmartPointer<vtkCollection> currentDvh = 
    vtkSmartPointer<vtkCollection>::Take( csvReadLogic->ReadCsvToTableNode(dvhCsvFileName) );
  vtkSmartPointer<vtkCollection> baselineDvh = 
    vtkSmartPointer<vtkCollection>::Take( csvReadLogic->ReadCsvToTableNode(baselineCsvFileName) );
 
  // Compare the current DVH to the baseline and determine mean and maximum difference
  agreementAcceptancePercentage = 0.0;
  int totalNumberOfBins = 0;
  int totalNumberOfAcceptedAgreements = 0;
  int numberOfAcceptedStructuresWith90 = 0;
  int numberOfAcceptedStructuresWith95 = 0;

  if (currentDvh->GetNumberOfItems() != baselineDvh->GetNumberOfItems())
  {
    std::cerr << "ERROR: Number of structures in the current and the baseline DVH tables do not match (" << currentDvh->GetNumberOfItems() << "<>" << baselineDvh->GetNumberOfItems() << ")" << std::endl;
    return 1;
  }

  for (int structureIndex=0; structureIndex < currentDvh->GetNumberOfItems(); structureIndex++)
  {
    vtkMRMLTableNode* currentStructure = vtkMRMLTableNode::SafeDownCast(currentDvh->GetItemAsObject(structureIndex));
    vtkMRMLTableNode* baselineStructure = vtkMRMLTableNode::SafeDownCast(baselineDvh->GetItemAsObject(structureIndex));
      
    // Calculate the agreement percentage for the current structure.
    double acceptedBinsRatio = vtkSlicerDoseVolumeHistogramComparisonLogic::CompareDvhTables(
      currentStructure, baselineStructure, nullptr, volumeDifferenceCriterion, doseToAgreementCriterion, maxDose );

    int numberOfBinsPerStructure = baselineStructure->GetTable()->GetNumberOfRows();
    totalNumberOfBins += numberOfBinsPerStructure;

    // Calculate the number of accepted bins in the structure based on the percent of accepted bins.
    int numberOfAcceptedAgreementsPerStructure = (int)(0.5 + (acceptedBinsRatio/100) * numberOfBinsPerStructure); 
    totalNumberOfAcceptedAgreements += numberOfAcceptedAgreementsPerStructure;

    if (acceptedBinsRatio > 90)
    {
      ++numberOfAcceptedStructuresWith90;

      if (acceptedBinsRatio > 95)
      {
        ++numberOfAcceptedStructuresWith95;
      }
    }
    
    std::string segmentId = currentStructure->GetAttribute(vtkSlicerDoseVolumeHistogramModuleLogic::DVH_SEGMENT_ID_ATTRIBUTE_NAME.c_str());
    std::ostringstream volumeAttributeNameStream;
    volumeAttributeNameStream << vtkMRMLDoseVolumeHistogramNode::DVH_ATTRIBUTE_PREFIX << vtkSlicerDoseVolumeHistogramModuleLogic::DVH_METRIC_TOTAL_VOLUME_CC;
    std::string structureVolume = currentStructure->GetAttribute(volumeAttributeNameStream.str().c_str());
    
    std::cout << "Accepted agreements per structure (" << segmentId << ", " << structureVolume << " cc): " << numberOfAcceptedAgreementsPerStructure
      << " out of " << numberOfBinsPerStructure << " (" << std::fixed << std::setprecision(2) << acceptedBinsRatio << "%)" << std::endl;
  } // for all structures

  std::cout << "Accepted structures with threshold of 90%: " << std::fixed << std::setprecision(2) << (double)numberOfAcceptedStructuresWith90 / (double)currentDvh->GetNumberOfItems() * 100.0 << std::endl;
  std::cout << "Accepted structures with threshold of 95%: " << std::fixed << std::setprecision(2) << (double)numberOfAcceptedStructuresWith95 / (double)currentDvh->GetNumberOfItems() * 100.0 << std::endl;

  agreementAcceptancePercentage = 100.0 * (double)totalNumberOfAcceptedAgreements / (double)totalNumberOfBins;
  
  return 0;
}

//-----------------------------------------------------------------------------
double GetAgreementForDvhPlotPoint(std::vector<std::pair<double,double> >& referenceDvhPlot, std::vector<std::pair<double,double> >& compareDvhPlot,
                               unsigned int compareIndex, double totalVolume, double maxDose,
                               double volumeDifferenceCriterion, double doseToAgreementCriterion)
{
  // Formula is (based on the article Ebert2010):
  //   gamma(i) = min{ Gamma[(di, vi), (dr, vr)] } for all {r=1..P}, where
  //   compareIndexth DVH point has dose di and volume vi
  //   P is the number of bins in the reference DVH, each rth bin having absolute dose dr and volume vr
  //   Gamma[(di, vi), (dr, vr)] = [ ( (100*(vr-vi)) / (volumeDifferenceCriterion * totalVolume) )^2 + ( (100*(dr-di)) / (doseToAgreementCriterion * maxDose) )^2 ] ^ 1/2
  //   volumeDifferenceCriterion is the volume-difference criterion (% of the total structure volume, totalVolume)
  //   doseToAgreementCriterion is the dose-to-agreement criterion (% of the maximum dose, maxDose)
  // A value of gamma(i) < 1 indicates agreement for the DVH bin compareIndex

  if (compareIndex >= compareDvhPlot.size())
  {
    std::cerr << "Invalid bin index for compare plot! (" << compareIndex << ">=" << compareDvhPlot.size() << ")" << std::endl;
    return -1.0;
  }

  double gamma = DBL_MAX;
  double di = compareDvhPlot[compareIndex].first;
  double vi = compareDvhPlot[compareIndex].second;

  std::vector<std::pair<double,double> >::iterator referenceDvhPlotIt;
  for (referenceDvhPlotIt = referenceDvhPlot.begin(); referenceDvhPlotIt != referenceDvhPlot.end(); ++referenceDvhPlotIt)
  {
    double dr = referenceDvhPlotIt->first;
    double vr = referenceDvhPlotIt->second;
    double currentGamma = sqrt( pow((100.0*(vr-vi))/(volumeDifferenceCriterion*totalVolume),2) + pow((100.0*(dr-di))/(doseToAgreementCriterion*maxDose),2) );
    if (currentGamma < gamma)
    {
      gamma = currentGamma;
    }
  }

  return gamma;
}

//-----------------------------------------------------------------------------
int CompareCsvDvhMetrics(std::string dvhMetricsCsvFileName, std::string baselineDvhMetricCsvFileName, double metricDifferenceThreshold)
{
  if (!vtksys::SystemTools::FileExists(baselineDvhMetricCsvFileName.c_str()))
  {
    std::cerr << "Loading baseline CSV DVH table from file '" << baselineDvhMetricCsvFileName << "' failed - the file does not exist" << std::endl;
    return 1;
  }

  std::vector<std::string> fieldNames;
  char currentLine[1024];
  char baselineLine[1024];

  std::ifstream currentStream;
  std::ifstream baselineStream;
  currentStream.open(dvhMetricsCsvFileName.c_str(), std::ifstream::in);
  baselineStream.open(baselineDvhMetricCsvFileName.c_str(), std::ifstream::in);

  bool firstLine = true;
  bool returnWithSuccess = true;

  while ( currentStream.getline(currentLine, 1024, '\n')
       && baselineStream.getline(baselineLine, 1024, '\n') )
  {
    std::string currentLineStr(currentLine);
    std::string baselineLineStr(baselineLine);

    size_t currentCommaPosition = currentLineStr.find(csvSeparatorCharacter);
    size_t baselineCommaPosition = baselineLineStr.find(csvSeparatorCharacter);

    // Collect field names
    if (firstLine)
    {
      while (currentCommaPosition != std::string::npos && baselineCommaPosition != std::string::npos)
      {
        fieldNames.push_back(currentLineStr.substr(0, currentCommaPosition));

        currentLineStr = currentLineStr.substr(currentCommaPosition+1);
        baselineLineStr = baselineLineStr.substr(baselineCommaPosition+1);

        currentCommaPosition = currentLineStr.find(csvSeparatorCharacter);
        baselineCommaPosition = baselineLineStr.find(csvSeparatorCharacter);
      }
      firstLine = false;
    }
    else
    {
      // Read all metrics from the current line
      int i=0;
      std::string structureName;
      while (currentCommaPosition != std::string::npos && baselineCommaPosition != std::string::npos)
      {
        if (i==0)
        {
          structureName = currentLineStr.substr(0, currentCommaPosition);
        }
        else
        {
          double currentMetric = vtkVariant(currentLineStr.substr(0, currentCommaPosition)).ToDouble();
          double baselineMetric = vtkVariant(baselineLineStr.substr(0, baselineCommaPosition)).ToDouble();

          double error = DBL_MAX;
          if (baselineMetric < EPSILON && currentMetric < EPSILON)
          {
            error = 0.0;
          }
          else
          {
            error = currentMetric / baselineMetric - 1.0;
          }

          if (error > metricDifferenceThreshold)
          {
            std::cerr << "Difference of metric '" << fieldNames[i] << "' for structure '" << structureName << "' is too high! Current=" << currentMetric << ", Baseline=" << baselineMetric << std::endl;
            returnWithSuccess = false;
          }
        }

        currentLineStr = currentLineStr.substr(currentCommaPosition+1);
        baselineLineStr = baselineLineStr.substr(baselineCommaPosition+1);
        currentCommaPosition = currentLineStr.find(csvSeparatorCharacter);
        baselineCommaPosition = baselineLineStr.find(csvSeparatorCharacter);

        i++;
      }
    }

    if ( (currentCommaPosition != std::string::npos) != (baselineCommaPosition != std::string::npos) )
    {
      std::cerr << "Number of fields differ in the current and the baseline metric tables" << std::endl;
      return 1;
    }
  }

  currentStream.close();
  baselineStream.close();

  if (!returnWithSuccess)
  {
    return 1;
  }

  return 0;
}
