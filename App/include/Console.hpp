#pragma once

#include <algorithm>
#include <array>
#include <string>
#include <utility>

// The devtools console's state, in one place.
//
// The text used to be a file-scope global, std::string consoletext[15], with
// slot 0 holding the line being typed and slots 1 to 14 the scrollback. It had
// no owner, and the shift-and-push that maintains it was copy-pasted into three
// places - GameTick.cpp and twice in ConsoleCmds.cpp, which flagged the
// duplication twice with a FIXME asking for exactly this class.
//
// It could not simply have moved into GameState: a std::string is neither
// trivially copyable nor trivially destructible, and GameStateTest asserts that
// GameState is both. So the console gets its own owner, and it takes the open
// flag and the cursor with it - those were GameState members describing state
// that lived in a global, which is the same incoherence one level down.
//
// std::array rather than std::string[15] because the element type has a
// non-trivial constructor and destructor. That is the case std::array exists
// for, and it keeps the magic 14 out of four call sites.
struct Console
{
	// Whether the console is open, and which character of the line the cursor is
	// on. Both were GameState members. `selected` stays unsigned because
	// Game::inputText takes a `unsigned*` for the cursor.
	bool open = false;
	unsigned selected = 0;

	// The cursor blink, which used to be two more GameState members. They
	// describe this cursor, so they came with it rather than being left behind
	// in GameState describing state that now lives elsewhere.
	bool blink = false;
	float blinkDelay = 0;

	// The line being typed, and what has been submitted before it.
	std::string line;
	std::array<std::string, 14> history;

	// Shifts the history up, dropping the oldest, and makes `text` the current
	// line. Used where the console speaks for itself - a map that would not
	// load, a texture that would not load - which is why it hands back what was
	// being typed: the message takes the line the typed text would have gone in,
	// and the typed text moves into the scrollback behind it. That is the
	// shift-and-push the three copy-pasted loops all did.
	void push(std::string text);

	// Shifts the history up, dropping the oldest, and clears the current line,
	// moving what was in it into the front of the history. Called once a command
	// has been dispatched from it, so the submitted line survives as scrollback
	// instead of being discarded with the line.
	//
	// An empty line is not a command and is not recorded: the original wrapped
	// the whole shift-and-clear in `if (!consoletext[0].empty())`, because
	// shifting a blank in would push an empty string over history[0] and lose
	// the newest entry, much as dispatch would push it one slot back.
	void submit();

private:
	// Makes history[0] free for the newest entry and moves every existing one a
	// slot toward the back, so the oldest falls off the end. copy_backward is
	// what makes this safe: a forward loop would copy slot k-1 over slot k and
	// then read the overwritten value on the next iteration.
	void historyShift();
};
