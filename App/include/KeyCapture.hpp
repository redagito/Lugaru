#pragma once

#include <atomic>

// KeyCapture owns the handshake between the one thread Lugaru has and the main
// loop. Clicking a keybind row in the controls menu parks the main loop and lets
// this thread block in SDL_WaitEvent until a key or a mouse button arrives; when
// one does, the thread writes the captured scancode into the caller's GameState
// and comes back. These three members are how the two sides agree that that is
// what is happening.
//
// They are atomics because they are genuinely shared: the thread writes them and
// the main thread polls them. As plain bool and int that is a data race by the
// standard, and it only behaved because the join happened to create the ordering
// edge after the fact - nothing ordered them during the window the main loop was
// actually polling.
//
// They cannot be members of GameState, which is the reason this is a struct of
// its own rather than a change to the member's type. std::atomic is not
// copyable, so one of these inside GameState would cost it the trivial
// copyability and trivial destructibility GameStateTest.cpp asserts - the two
// properties that let GameState be built and destroyed in a unit test with no GL
// context. Constructed once in main() beside GameState, passed by reference down
// the same chain GameState already travels. There is deliberately no accessor and
// no shared instance: a static owner behind a getter is the global it replaces,
// with the storage hidden instead of removed.
//
// waiting is not only the capture thread's flag. Game::inputText uses it as well,
// to mean that SDL text input has been started and has not yet been cancelled or
// submitted, which is why main() skips its SDL_PollEvent while it is set. The two
// uses are mutually exclusive in practice - one is the controls menu, the other
// the name and console fields - and they were one flag before this struct existed
// too, so they are one flag now.

struct KeyCapture
{
	// Whether input is being collected: either the key-capture thread is parked,
	// or Game::inputText has started SDL text input.
	std::atomic<bool> waiting = false;

	// Which keybind row the controls menu is waiting for a key on; -1 means none.
	std::atomic<int> keyselect = 0;

	// Set by the capture thread once it has written the captured scancode, to ask
	// the main thread to rebuild the menu that displays it. The thread used to do
	// that rebuild itself, which put it in conflict with the file-static
	// Menu::items the main thread walks every frame, and with the GameAssets
	// textures Menu::Load reads on its way through.
	std::atomic<bool> reloadRequested = false;

	// The key the thread captured, and which row of the controls menu was waiting
	// for one. The thread parks them here instead of writing the keybind itself:
	// the ten keybind members are plain unsigned short rather than atomic, so
	// assigning one from the thread raced every main-thread read of it. The main
	// thread moves these into the keybind when it answers the reload request.
	// -1 is the "nothing captured" sentinel, and SDL_SCANCODE_ESCAPE means the
	// capture was cancelled, which the main thread treats as no change at all.
	std::atomic<int> capturedScancode = -1;
	std::atomic<int> capturedRow = -1;

	// Whether a reload is owed, clearing the request as it answers it.
	//
	// Both sides are seq_cst, the default, so this read-modify-write acquires
	// whatever the releasing store sequenced before it - which is the captured
	// keybind write, and every other write the thread made before it gave up.
	// That is what lets the main thread go on to rebuild the menu from a keybind
	// value it did not write itself. exchange(false) rather than a plain load
	// followed by a store, so two takes in a row cannot both report the same
	// request and rebuild the menu twice from one rebind.
	bool takeReloadRequest()
	{
		return reloadRequested.exchange(false);
	}
};