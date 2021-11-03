/*=========================================================================

   Program: ParaView
   Module:  pqArrayListWidget.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
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

========================================================================*/
#include "pqArrayListWidget.h"

#include "pqAnnotationsModel.h"
#include "pqExpandableTableView.h"
#include "pqHeaderView.h"

#include <QEvent>
#include <QScopedValueRollback>
#include <QVBoxLayout>

class pqArrayListModel : public pqAnnotationsModel
{
  typedef pqAnnotationsModel Superclass;

public:
  pqArrayListModel(pqArrayListWidget* parent = nullptr)
    : Superclass(parent)
    , ParentWidget(parent)
  {
  }

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override
  {
    // TODO add icon for array type ?
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
      switch (section)
      {
        case VALUE:
          return this->ArrayLabel;
        case LABEL:
          return tr("New Name");
        default:
          break;
      }
    }

    return this->Superclass::headerData(section, orientation, role);
  }

  Qt::ItemFlags flags(const QModelIndex& idx) const override
  {
    auto value = this->Superclass::flags(idx);
    if (idx.column() == VALUE)
    {
      return value & ~Qt::ItemIsEditable;
    }

    return value;
  }

  QString ArrayLabel = tr("Array Name");
  pqArrayListWidget* ParentWidget = nullptr;
};

pqArrayListWidget::pqArrayListWidget(QWidget* parent)
  : Superclass(parent)
{
  auto lay = new QVBoxLayout(this);
  this->TableView = new pqExpandableTableView(this);
  lay->addWidget(this->TableView);
  lay->setMargin(0);
  lay->setSpacing(2);

  pqHeaderView* myheader = new pqHeaderView(Qt::Horizontal, this->TableView);
  this->TableView->setHorizontalHeader(myheader);
  myheader->setHighlightSections(false);
  myheader->setSectionResizeMode(QHeaderView::ResizeToContents);
  myheader->setStretchLastSection(true);

  this->Model = new pqArrayListModel(this);
  this->Model->setSupportsReorder(true);

  this->TableView->setModel(this->Model);
  this->TableView->setSortingEnabled(true);
  this->TableView->setColumnHidden(pqAnnotationsModel::COLOR, true);
  this->TableView->setColumnHidden(pqAnnotationsModel::OPACITY, true);
  this->TableView->setColumnHidden(pqAnnotationsModel::VISIBILITY, true);

  this->connect(this->TableView, SIGNAL(widgetModified), SLOT(updateProperty));
  QObject::connect(
    this->Model, &pqArrayListModel::dataChanged, this, &pqArrayListWidget::updateProperty);
}

//-----------------------------------------------------------------------------
void pqArrayListWidget::setMaximumRowCountBeforeScrolling(int size)
{
  this->TableView->setMaximumRowCountBeforeScrolling(size);
}

//-----------------------------------------------------------------------------
bool pqArrayListWidget::event(QEvent* evt)
{
  if (evt->type() == QEvent::DynamicPropertyChange && !this->UpdatingProperty)
  {
    auto devt = dynamic_cast<QDynamicPropertyChangeEvent*>(evt);
    this->propertyChanged(devt->propertyName());
    return true;
  }

  return this->Superclass::event(evt);
}

//-----------------------------------------------------------------------------
void pqArrayListWidget::setHeaderLabel(const QString& label)
{
  this->Model->ArrayLabel = label;
}

//-----------------------------------------------------------------------------
void pqArrayListWidget::propertyChanged(const QString& pname)
{
  if (this->UpdatingProperty)
  {
    return;
  }

  QVariant property = this->property(pname.toUtf8().data());
  const QList<QVariant> status_values = property.value<QList<QVariant>>();
  std::vector<std::pair<QString, QString>> annotations;
  for (auto idx = 0; idx < status_values.size() - 1; idx += 2)
  {
    annotations.emplace_back(status_values[idx].toString(), status_values[idx + 1].toString());
  }

  this->Model->setAnnotations(annotations, true);
}

//-----------------------------------------------------------------------------
void pqArrayListWidget::updateProperty()
{
  // for scope
  {
    QScopedValueRollback<bool> rollback(this->UpdatingProperty, true);
    QList<QVariant> values;
    std::vector<std::pair<QString, QString>> annotations = this->Model->annotations();
    for (auto annotation : annotations)
    {
      values.push_back(annotation.first);
      values.push_back(annotation.second);
    }

    this->setProperty(this->Model->ArrayLabel.toLatin1(), QVariant::fromValue(values));
  }

  Q_EMIT this->widgetModified();
}
