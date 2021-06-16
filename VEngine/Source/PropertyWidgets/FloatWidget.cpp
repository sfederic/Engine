#include "FloatWidget.h"
#include <QHBoxLayout>
#include <QLabel>

FloatWidget::FloatWidget(float* value, QWidget* parent) : QDoubleSpinBox(parent)
{
	_value = value;

	setDecimals(4);
	setMinimum(std::numeric_limits<float>::lowest());
	setMaximum(std::numeric_limits<double>::max());
	setButtonSymbols(QAbstractSpinBox::NoButtons);

	connect(this, &QDoubleSpinBox::editingFinished, this, &FloatWidget::SetValue);
}

void FloatWidget::SetValue()
{
	if (_value)
	{
		*_value = (float)value();
	}
}
