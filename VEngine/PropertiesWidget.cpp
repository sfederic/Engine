#include "PropertiesWidget.h"
#include "WorldEditor.h"
#include <qgridlayout.h>
#include <qlabel.h>

void PropertiesWidget::SetActorPositionX()
{
    Actor* picked = gWorldEditor.pickedActor;
    if (picked && posEditX)
    {
        XMFLOAT3 pos = picked->GetPositionFloat3();
        pos.x = posEditX->text().toFloat();
        gWorldEditor.pickedActor->SetPosition(pos);
    }

    clearFocus();
}

void PropertiesWidget::SetActorPositionY()
{
    Actor* picked = gWorldEditor.pickedActor;
    if (picked && posEditY)
    {
        XMFLOAT3 pos = picked->GetPositionFloat3();
        pos.y = posEditY->text().toFloat();
        gWorldEditor.pickedActor->SetPosition(pos);
    }

    clearFocus();
}

void PropertiesWidget::SetActorPositionZ()
{
    Actor* picked = gWorldEditor.pickedActor;
    if (picked && posEditZ)
    {
        XMFLOAT3 pos = picked->GetPositionFloat3();
        pos.z = posEditZ->text().toFloat();
        gWorldEditor.pickedActor->SetPosition(pos);
    }

    clearFocus();
}

void PropertiesWidget::SetActorScaleX()
{
    Actor* picked = gWorldEditor.pickedActor;
    if (picked && scaleEditX)
    {
        XMFLOAT3 scale = picked->GetScale();
        scale.x = posEditX->text().toFloat();
        gWorldEditor.pickedActor->SetScale(scale);
    }
}

void PropertiesWidget::SetActorScaleY()
{
    Actor* picked = gWorldEditor.pickedActor;
    if (picked && scaleEditY)
    {
        XMFLOAT3 scale = picked->GetScale();
        scale.y = posEditY->text().toFloat();
        gWorldEditor.pickedActor->SetScale(scale);
    }
}

void PropertiesWidget::SetActorScaleZ()
{
    Actor* picked = gWorldEditor.pickedActor;
    if (picked && scaleEditZ)
    {
        XMFLOAT3 scale = picked->GetScale();
        scale.z = posEditZ->text().toFloat();
        gWorldEditor.pickedActor->SetScale(scale);
    }
}

void PropertiesWidget::SetActorRotationX()
{
}

void PropertiesWidget::SetActorRotationY()
{
}

void PropertiesWidget::SetActorRotationZ()
{
}

PropertiesWidget::PropertiesWidget(QWidget* parent) : QWidget(parent)
{
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    QGridLayout* grid = new QGridLayout();

    //Position
    QLabel* posLabel = new QLabel("Position");

    posEditX = new TransformEditWidget(0.f, this);
    connect(posEditX, &QLineEdit::returnPressed, this, &PropertiesWidget::SetActorPositionX);
    posEditY = new TransformEditWidget(0.f, this);
    connect(posEditY, &QLineEdit::returnPressed, this, &PropertiesWidget::SetActorPositionY);
    posEditZ = new TransformEditWidget(0.f, this);
    connect(posEditZ, &QLineEdit::returnPressed, this, &PropertiesWidget::SetActorPositionZ);

    grid->addWidget(posLabel, 0, 0);
    grid->addWidget(posEditX, 0, 1);
    grid->addWidget(posEditY, 0, 2);
    grid->addWidget(posEditZ, 0, 3);

    //Rotation
    rotEditX = new TransformEditWidget(0.0f, this);
    rotEditY = new TransformEditWidget(0.0f, this);
    rotEditZ = new TransformEditWidget(0.0f, this);

    QLabel* rotLabel = new QLabel("Rotation");

    grid->addWidget(rotLabel, 1, 0);
    grid->addWidget(rotEditX, 1, 1);
    grid->addWidget(rotEditY, 1, 2);
    grid->addWidget(rotEditZ, 1, 3);

    //Scale
    scaleEditX = new TransformEditWidget(0.0f, this);
    connect(scaleEditX, &QLineEdit::returnPressed, this, &PropertiesWidget::SetActorScaleX);
    scaleEditY = new TransformEditWidget(0.0f, this);
    connect(scaleEditY, &QLineEdit::returnPressed, this, &PropertiesWidget::SetActorScaleY);
    scaleEditZ = new TransformEditWidget(0.0f, this);
    connect(scaleEditZ, &QLineEdit::returnPressed, this, &PropertiesWidget::SetActorScaleZ);


    QLabel* scaleLabel = new QLabel("Scale");

    grid->addWidget(scaleLabel, 2, 0);
    grid->addWidget(scaleEditX, 2, 1);
    grid->addWidget(scaleEditY, 2, 2);
    grid->addWidget(scaleEditZ, 2, 3);

    grid->setAlignment(Qt::AlignTop);

    QHBoxLayout* hLayoutTop = new QHBoxLayout();

    hLayoutTop->addLayout(grid);

    setLayout(hLayoutTop);
}

void PropertiesWidget::Tick()
{
    Actor* picked = gWorldEditor.pickedActor;
    if (posEditX->hasFocus())
    {
        return;
    }
    else if (picked)
    {
        XMFLOAT3 pos = picked->GetPositionFloat3();
        posEditX->setText(QString::number(pos.x));
        posEditY->setText(QString::number(pos.y));
        posEditZ->setText(QString::number(pos.z));

        XMFLOAT3 rot = picked->GetRollPitchYaw();
        rotEditX->setText(QString::number(rot.x));
        rotEditY->setText(QString::number(rot.y));
        rotEditZ->setText(QString::number(rot.z));

        XMFLOAT3 scale = picked->GetScale();
        scaleEditX->setText(QString::number(scale.x));
        scaleEditY->setText(QString::number(scale.y));
        scaleEditZ->setText(QString::number(scale.z));
    }
}