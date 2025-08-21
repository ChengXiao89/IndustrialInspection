#include "status_table.h"
#include <QHeaderView>

status_table::status_table(QWidget* parent)
	:QTableWidget(parent)
{
	
}

status_table::~status_table()
{
	
}

void status_table::initialize()
{
	setMinimumSize(120, 180);
	setSelectionMode(QAbstractItemView::NoSelection);
	horizontalHeader()->setStretchLastSection(true);
	verticalHeader()->setVisible(false);
	horizontalHeader()->setVisible(false);
}