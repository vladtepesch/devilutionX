/**
 * @file questsbook.h
 *
 * Interface of questlog handling
 */
#pragma once

#include <cstdint>
#include <memory>
#include "control.h"
#include "engine/surface.hpp"
#include "utils/language.h"

namespace devilution {

/// handles the quest book dialog content
class QuestBook {
public:
	/// returns quest book singleton
	static QuestBook &instance();

	/// collects the quest data to display and sets the book as opened
	void open();
	/// closes the book
	void close();

	/// should replace the `QuestLogIsOpen`
	bool isOpen() const;

	/// moves the selection marker up
	void selectionUp();
	/// moves the selection marker down
	void selectionDown();

	/// draws the book to the surface
	void draw(const Surface &surf);

	///processes a click within the quest log pane
	void processClick();

	///plays the currently selected entry (if any) and closes the book
	void playEntry();

private:
	// enforce singleton
	QuestBook();
	~QuestBook();
	QuestBook(QuestBook &&op) noexcept;
	QuestBook &operator=(QuestBook &&op) noexcept;

	struct QuestBookPimpl;
	std::unique_ptr<QuestBookPimpl> p;
};

} // namespace devilution
