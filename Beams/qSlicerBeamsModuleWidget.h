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

#ifndef __qSlicerBeamsModuleWidget_h
#define __qSlicerBeamsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerBeamsModuleExport.h"

class qSlicerBeamsModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLRTBeamNode;

/// \ingroup SlicerRt_QtModules_Beams
class Q_SLICER_QTMODULES_BEAMS_EXPORT qSlicerBeamsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerBeamsModuleWidget(QWidget *parent=nullptr);
  virtual ~qSlicerBeamsModuleWidget();

  virtual void enter();

  /// Support of node editing. Selects node in user interface that the user wants to edit
  virtual bool setEditedNode(vtkMRMLNode* node, QString role=QString(), QString context=QString());
  /// Return a higher confidence value (0.6) for beam nodes to prevent beams to be opened by Models
  virtual double nodeEditable(vtkMRMLNode* node);

public slots:
  /// Set the current MRML scene to the widget
  virtual void setMRMLScene(vtkMRMLScene*);

  /// Process loaded scene
  void onSceneImportedEvent();

  /// Update the entire widget based on the current parameter node
  void updateWidgetFromMRML();

// Update functions
protected slots:
  /// Updates button states
  void updateButtonsState();
  /// Logic modified
  void onLogicModified();

// UI change handlers
protected slots:
  /// Handles active beam selection changed event
  void setBeamNode(vtkMRMLNode* node);
  /// Currently selected RTBeam was modified
  void onBeamNodeModified();

  /// Handle switch to parent plan button click, takes the user to External Beam Planning and selects parent plan
  void switchToParentPlanButtonClicked();

  // Main parameters
  void beamNameChanged(const QString &);
  void beamWeightChanged(double);

protected:
  QScopedPointer<qSlicerBeamsModuleWidgetPrivate> d_ptr;

protected:
  virtual void setup();
  void onEnter();

private:
  Q_DECLARE_PRIVATE(qSlicerBeamsModuleWidget);
  Q_DISABLE_COPY(qSlicerBeamsModuleWidget);
};

#endif
