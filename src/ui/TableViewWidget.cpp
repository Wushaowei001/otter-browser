/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2014 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "TableViewWidget.h"

#include <QtCore/QTimer>
#include <QtGui/QDropEvent>

namespace Otter
{

TableViewWidget::TableViewWidget(QWidget *parent) : QTableView(parent),
	m_model(NULL),
	m_dropRow(-1),
	m_isModified(false)
{
	viewport()->setAcceptDrops(true);
}

void TableViewWidget::dropEvent(QDropEvent *event)
{
	QTableView::dropEvent(event);

	if (event->isAccepted())
	{
		m_dropRow = indexAt(event->pos()).row();

		if (dropIndicatorPosition() == QAbstractItemView::BelowItem)
		{
			++m_dropRow;
		}

		m_isModified = true;

		emit modified();

		QTimer::singleShot(50, this, SLOT(updateDropSelection()));
	}
}

void TableViewWidget::moveRow(bool up)
{
	if (!m_model)
	{
		return;
	}

	const int sourceRow = currentIndex().row();
	const int destinationRow = (up ? (sourceRow - 1) : (sourceRow + 1));

	if ((up && sourceRow > 0) || (!up && sourceRow < (m_model->rowCount() - 1)))
	{
		m_model->insertRow(sourceRow, m_model->takeRow(destinationRow));

		setCurrentIndex(getIndex(destinationRow, 0));
		notifySelectionChanged();

		m_isModified = true;

		emit modified();
	}
}

void TableViewWidget::insertRow(const QList<QStandardItem*> &items)
{
	if (!m_model)
	{
		return;
	}

	const int row = (currentIndex().row() + 1);

	if (items.count() > 0)
	{
		m_model->insertRow(row, items);
	}
	else
	{
		m_model->insertRow(row);
	}

	setCurrentIndex(getIndex(row, 0));

	m_isModified = true;

	emit modified();
}

void TableViewWidget::removeRow()
{
	if (!m_model)
	{
		return;
	}

	const int row = currentIndex().row();

	if (row >= 0)
	{
		m_model->removeRow(row);

		m_isModified = true;

		emit modified();
	}
}

void TableViewWidget::moveUpRow()
{
	moveRow(true);
}

void TableViewWidget::moveDownRow()
{
	moveRow(false);
}

void TableViewWidget::notifySelectionChanged()
{
	if (m_model)
	{
		emit canMoveUpChanged(canMoveUp());
		emit canMoveDownChanged(canMoveDown());
		emit needsActionsUpdate();
	}
}

void TableViewWidget::updateDropSelection()
{
	setCurrentIndex(getIndex(qBound(0, m_dropRow, getRowCount()), 0));

	m_dropRow = -1;
}

void TableViewWidget::setFilter(const QString filter)
{
	if (!m_model)
	{
		return;
	}

	for (int i = 0; i < m_model->rowCount(); ++i)
	{
		bool found = filter.isEmpty();

		if (!found)
		{
			for (int j = 0; j < m_model->columnCount(); ++j)
			{
				QStandardItem *item = m_model->item(i, j);

				if (item && item->text().contains(filter, Qt::CaseInsensitive))
				{
					found = true;

					break;
				}
			}
		}

		setRowHidden(i, !found);
	}
}

void TableViewWidget::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (m_model)
	{
		m_model->setData(index, value, role);
	}
}

void TableViewWidget::setModel(QAbstractItemModel *model)
{
	QTableView::setModel(model);

	if (!model)
	{
		return;
	}

	model->setParent(this);

	if (model->inherits(QStringLiteral("QStandardItemModel").toLatin1()))
	{
		m_model = qobject_cast<QStandardItemModel*>(model);
	}

	connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(notifySelectionChanged()));
	connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SIGNAL(modified()));
}

QStandardItemModel* TableViewWidget::getModel()
{
	return m_model;
}

QModelIndex TableViewWidget::getIndex(int row, int column) const
{
	return (m_model ? m_model->index(row, column) : QModelIndex());
}

int TableViewWidget::getCurrentRow() const
{
	return (selectionModel()->hasSelection() ? currentIndex().row() : -1);
}

int TableViewWidget::getRowCount() const
{
	return (m_model ? m_model->rowCount() : 0);
}

int TableViewWidget::getColumnCount() const
{
	return (m_model ? m_model->columnCount() : 0);
}

bool TableViewWidget::canMoveUp() const
{
	return (currentIndex().row() > 0 && m_model->rowCount() > 1);
}

bool TableViewWidget::canMoveDown() const
{
	const int currentRow = currentIndex().row();

	return (currentRow >= 0 && m_model->rowCount() > 1 && currentRow < (m_model->rowCount() - 1));
}

bool TableViewWidget::isModified() const
{
	return m_isModified;
}

}
