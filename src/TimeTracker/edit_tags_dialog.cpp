#include "edit_tags_dialog.h"
#include "ui_edit_tags_dialog.h"

#include "common.h"

#include <QColorDialog>

EditTagsDialog::EditTagsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTagsDialog),
    m_tagListModel(new TagListModel)
{
    ui->setupUi(this);

    if (!g_app.database()->loadAllTags())
    {
        Q_ASSERT(false); // crash
        QApplication::exit(-1);
    }

    for (auto tag : g_app.database()->getCachedTags())
    {
        m_tagListModel->addTag(tag);
    }

    ui->tagListView->setModel(m_tagListModel);

    connect(ui->tagListView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &EditTagsDialog::tagListViewSelectionChanged);
    connect(ui->tagColorPickButton, &QPushButton::clicked,
            this, &EditTagsDialog::tagColorPickButtonClicked);
}

QString rgbToHex(qint64 rgb)
{
// @Incomplete
    return QString::number(rgb & 0xFFFFFF00);
}

void EditTagsDialog::tagListViewSelectionChanged(const QItemSelection& selection)
{
    if (m_editTag != nullptr)
    {
        if (g_app.database()->saveTag(m_editTag))
        {
            APP_ERRSTREAM << "tag" << m_editTag->name << "edited successfully";
        }
        else
        {
            APP_ERRSTREAM << "tag" << m_editTag->name << "edit FAILURE!!!!!!!!11111111";
        }
    }

    QModelIndexList indexes = selection.indexes();
    if (indexes.count() == 0)
        return;

    const QModelIndex& index = indexes.at(0);
    if (!index.isValid())
    {
        m_editTag = nullptr;
        ui->tagEditFrame->setEnabled(false);
        ui->tagNameLineEdit->setText("");
        ui->tagNoteLineEdit->setText("");
        ui->tagColorPickButton->setText("No Color");
    }
    else
    {
        Tag* tag = m_tagListModel->tagAtRow(index.row());
        if (tag == nullptr)
        {
            APP_ERRSTREAM << "weird";
            ui->tagEditFrame->setEnabled(false);
            return;
        }

        ui->tagNameLineEdit->setText(tag->name);
        ui->tagNoteLineEdit->setText(tag->note);
        ui->tagColorPickButton->setText(rgbToHex(tag->color));

        m_editTag = tag;
        ui->tagEditFrame->setEnabled(true);
    }
}

void EditTagsDialog::tagColorPickButtonClicked(bool checked)
{
    Q_UNUSED(checked);

    if (!m_editTag)
    {
        APP_ERRSTREAM << "This control should be disabled!";
        return;
    }

    QColor tagColor = QColor::fromRgb((QRgb)m_editTag->color);
    QColor color = QColorDialog::getColor(tagColor, this, "Select Tag Color", QColorDialog::DontUseNativeDialog);

    if (!color.isValid())
    {
        return;
    }

    m_editTag->color = color.rgb();
    ui->tagColorPickButton->setText(rgbToHex(m_editTag->color));
}

EditTagsDialog::~EditTagsDialog()
{
    delete ui;
    delete m_tagListModel;
}
