/*=========================================================================

   Program: ParaView
   Module: pqColorEditorPropertyWidget.h

   Copyright (c) 2005-2012 Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "pqColorEditorPropertyWidget.h"

#include <QGroupBox>
#include <QVBoxLayout>

#include "vtkSMProxy.h"
#include "vtkSMPropertyHelper.h"


#include "pqProxy.h"
#include "pqRepresentation.h"
#include "pqApplicationCore.h"
#include "pqServerManagerModel.h"
#include "pqGenericSummaryDisplayPanel.h"

pqColorEditorPropertyWidget::pqColorEditorPropertyWidget(vtkSMProxy *smProxy,
  QWidget *parentObject)
  : pqPropertyWidget(smProxy, parentObject)
{
  QVBoxLayout *layoutLocal = new QVBoxLayout;
  layoutLocal->setMargin(2);
  QGroupBox *groupBox = new QGroupBox("Color");
  QVBoxLayout *groupBoxLayout = new QVBoxLayout;
  groupBoxLayout->setMargin(0);

  pqServerManagerModel *smm = pqApplicationCore::instance()->getServerManagerModel();
  pqProxy *pqproxy = smm->findItem<pqProxy *>(smProxy);

  pqRepresentation *representation = qobject_cast<pqRepresentation *>(pqproxy);

  if(smProxy->GetProperty("Representation") != NULL)
    {
    QList<pqGenericSummaryDisplayPanel::DisplayAttributes> attributes;
    attributes.append(pqGenericSummaryDisplayPanel::ColorBy);
    QWidget *colorWidget =
      new pqGenericSummaryDisplayPanel(representation, attributes);
    groupBoxLayout->addWidget(colorWidget);
    }

  groupBox->setLayout(groupBoxLayout);

  layoutLocal->addWidget(groupBox);
  setLayout(layoutLocal);

  setShowLabel(false);
}

pqColorEditorPropertyWidget::~pqColorEditorPropertyWidget()
{
}
