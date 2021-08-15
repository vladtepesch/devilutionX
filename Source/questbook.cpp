/**
 * @file questsbook.cpp
 *
 * implementation of questlog handling
 */

#include "questbook.hpp"

#include "diablo.h"
#include "quests.h"
#include "engine/rectangle.hpp"
#include "engine/render/cel_render.hpp"
#include "engine/render/text_render.hpp"
#include "minitext.h"

namespace devilution {

/// quest log private implementation
struct QuestBook::QuestBookPimpl {
public:
	void open();
	void close();

	/// should replace the `QuestLogIsOpen`
	bool isOpen() const;

	void selectionUp();
	void selectionDown();

	void draw(const Surface &surf);

	void processClick();

	void playEntry();

private:
	void selectQuestsToDislay();
	void calculateLayout();
	void PrintQLString(const Surface &out, int x, int y, const char *str, bool marked, bool disabled = false) const;
	int cursorToSelection() const;

	void setOpen(bool v);

	static constexpr Rectangle panelInnerRect { { 32, 40 }, { 280, 290 } }; //inner rect omits the "Quest Log" caption line
	static constexpr int lineHeight = 12;
	static constexpr int maxSpacing = lineHeight * 2;
	static constexpr int defaultTop = 12;
	static constexpr int defaultCloseY = panelInnerRect.size.height - lineHeight - 7;

	/// \todo should replace global `QuestLogIsOpen` (together with `isOpen`)
	bool opened = false;

	int topY = defaultTop;
	int lineSpacing = lineHeight;
	int act2finSpacing = lineHeight;
	int closeY = defaultCloseY;

	int qlist[MAXQUESTS];   ///< indices of quests to display in quest log window. `fistfinishedEntry` are active quests the rest are completed
	int qlistCnt;           ///< overall number of qlist entries
	int firstFinishedEntry; ///< first (nonselectable) finished quest in list
	int selectedEntry;      ///< currently selected quest list item
};

int QuestBook::QuestBookPimpl::cursorToSelection() const
{
	Rectangle innerArea = panelInnerRect;
	innerArea.position += Displacement(LeftPanel.position.x, LeftPanel.position.y);
	if (!innerArea.Contains(MousePosition))
		return -1;
	int y = MousePosition.y - innerArea.position.y;
	for (int i = 0; i < firstFinishedEntry; i++) {
		if ((y >= topY + i * lineSpacing)
		    && (y < topY + i * lineSpacing + lineHeight)) {
			return i;
		}
	}
	if ((y >= closeY)
	    && (y < closeY + lineHeight)) {
		return qlistCnt;
	}
	return -1;
}

void QuestBook::QuestBookPimpl::playEntry()
{
	PlaySFX(IS_TITLSLCT);
	if (qlistCnt != 0 && selectedEntry < firstFinishedEntry)
		InitQTextMsg(Quests[qlist[selectedEntry]]._qmsg);
	close();
}

void QuestBook::QuestBookPimpl::selectQuestsToDislay()
{
	auto sortQuestIdx = [](int a, int b) {
		return QuestData[a].questBookOrder < QuestData[b].questBookOrder;
	};

	qlistCnt = 0;
	for (auto &quest : Quests) {
		if (quest._qactive == QUEST_ACTIVE && quest._qlog) {
			qlist[qlistCnt] = quest._qidx;
			qlistCnt++;
		}
	}
	firstFinishedEntry = qlistCnt;
	for (auto &quest : Quests) {
		if (quest._qactive == QUEST_DONE || quest._qactive == QUEST_HIVE_DONE) {
			qlist[qlistCnt] = quest._qidx;
			qlistCnt++;
		}
	}

	std::sort(&qlist[0], &qlist[firstFinishedEntry], sortQuestIdx);
	std::sort(&qlist[firstFinishedEntry], &qlist[qlistCnt], sortQuestIdx);
}

void QuestBook::QuestBookPimpl::calculateLayout()
{
	closeY = defaultCloseY;
	topY = defaultTop;
	act2finSpacing = lineHeight / 2;

	int overallMinHeight = qlistCnt * lineHeight + act2finSpacing;
	int space = (closeY - topY - lineHeight);

	if (qlistCnt > 0) {
		if (qlistCnt < 21) {
			int additionalSpace = (space - overallMinHeight);
			int addLineSpacing = additionalSpace / qlistCnt;
			int addSepSpacint = additionalSpace - (addLineSpacing * qlistCnt);
			lineSpacing = std::min(maxSpacing, lineHeight + addLineSpacing);
			act2finSpacing += addSepSpacint;

			int overallHeight = qlistCnt * lineSpacing + act2finSpacing;
			topY += (space - overallHeight) / 3;
		} else {
			lineSpacing = lineHeight - 1;
			act2finSpacing = 4;
			if (qlistCnt == 23) {
				topY /= 2;
			} else if (qlistCnt == 24) {
				topY /= 4;
				act2finSpacing /= 2;
			}
		}
	}
}

void QuestBook::QuestBookPimpl::PrintQLString(const Surface &out, int x, int y, const char *str, bool marked, bool disabled) const
{
	int width = GetLineWidth(str);
	int sx = x + std::max((257 - width) / 2, 0);
	int sy = y + lineHeight; //seems that DrawString y is the text base line -> so add a lines height
	if (marked) {
		CelDrawTo(out, GetPanelPosition(UiPanels::Quest, { sx - 20, sy + 1 }), *pSPentSpn2Cels, PentSpn2Spin());
	}
	DrawString(out, str, { GetPanelPosition(UiPanels::Quest, { sx, sy }), { 257, 0 } }, disabled ? UiFlags::ColorGold : UiFlags::ColorSilver);
	if (marked) {
		CelDrawTo(out, GetPanelPosition(UiPanels::Quest, { sx + width + 7, sy + 1 }), *pSPentSpn2Cels, PentSpn2Spin());
	}
}

void QuestBook::QuestBookPimpl::open()
{
	selectQuestsToDislay();
	calculateLayout();
	selectedEntry = (firstFinishedEntry == 0) ? qlistCnt : 0;
	setOpen(true);
}

void QuestBook::QuestBookPimpl::close()
{
	setOpen(false);
}

void QuestBook::QuestBookPimpl::setOpen(bool v)
{
	QuestLogIsOpen = v;
	opened = v;
}

bool QuestBook::QuestBookPimpl::isOpen() const
{
	return QuestLogIsOpen;
}

void QuestBook::QuestBookPimpl::selectionUp()
{
	if (qlistCnt != 0) {
		if (selectedEntry == 0 || (firstFinishedEntry == 0)) {
			selectedEntry = qlistCnt;
		} else if (selectedEntry >= firstFinishedEntry) {
			selectedEntry = firstFinishedEntry - 1;
		} else {
			selectedEntry--;
		}

		PlaySFX(IS_TITLEMOV);
	}
}

void QuestBook::QuestBookPimpl::selectionDown()
{
	if (qlistCnt != 0) {
		if (selectedEntry == qlistCnt) {
			selectedEntry = 0;
		} else {
			selectedEntry++;
		}
		if (selectedEntry >= firstFinishedEntry) {
			selectedEntry = qlistCnt;
		}
		PlaySFX(IS_TITLEMOV);
	}
}

void QuestBook::QuestBookPimpl::draw(const Surface &out)
{
	int l = cursorToSelection();
	if (l >= 0) {
		selectedEntry = l;
	}
	const auto x = panelInnerRect.position.x;
	CelDrawTo(out, GetPanelPosition(UiPanels::Quest, { 0, 351 }), *pQLogCel, 1);
	PrintQLString(out, x, panelInnerRect.position.y - lineHeight, _("Quest Log"), false, true);
	int y = panelInnerRect.position.y + topY;
	for (int i = 0; i < qlistCnt; i++) {
		if (i == firstFinishedEntry) {
			y += act2finSpacing;
		}
		PrintQLString(out, x, y, _(QuestData[qlist[i]]._qlstr), i == selectedEntry, i >= firstFinishedEntry);
		y += lineSpacing;
	}
	PrintQLString(out, x, panelInnerRect.position.y + closeY, _("Close Quest Log"), selectedEntry == qlistCnt);
}

void QuestBook::QuestBookPimpl::processClick()
{
	int l = cursorToSelection();
	if (l != -1) {
		QuestlogEnter();
	}
}

QuestBook &QuestBook::instance()
{
	static QuestBook s;
	return s;
}

void QuestBook::open()
{
	p->open();
}

void QuestBook::close()
{
	p->close();
}

bool QuestBook::isOpen() const
{
	return p->isOpen();
}

void QuestBook::selectionUp()
{
	p->selectionUp();
}

void QuestBook::selectionDown()
{
	p->selectionDown();
}

void QuestBook::draw(const Surface &surf)
{
	p->draw(surf);
}

void QuestBook::processClick()
{
	p->processClick();
}

void QuestBook::playEntry()
{
	p->playEntry();
}

QuestBook::QuestBook()
    : p(new QuestBookPimpl)
{
}
//has to be done here to get unique_ptr working with incomplete type
QuestBook::~QuestBook() = default;
QuestBook::QuestBook(QuestBook &&) noexcept = default;
QuestBook &QuestBook::operator=(QuestBook &&) noexcept = default;

} // namespace devilution
