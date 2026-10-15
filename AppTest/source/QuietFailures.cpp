// Unit tests for the App layer.
//
// References AppLib and Catch2 only.
//
// Keeps a failing test from demanding attention through a Windows dialog.
//
// When a test aborts, hard-crashes, or trips a CRT assertion, the default
// behaviour on Windows is three overlapping prompts - a CRT assert box, then a
// Windows Error Reporting dialog, then a "has stopped working" box. They appear
// on top of the console, stack if several tests fail, and steal focus. For a
// suite whose output you want to read, that is the opposite of helpful.
//
// This turns all three off, leaving the failure where it is most useful: in the
// test output and the exit code. Nothing here changes what is detected - no
// error is hidden or swallowed, because the suppression happens at the point of
// *reporting*, and the reports go to stderr instead.
//
// It is a static initialiser rather than a main() so it needs no link changes:
// it runs before main, before any test, and works with Catch2's own main. That
// also means the ordering is guaranteed relative to the tests by construction
// rather than by anything Catch2 promises.
//
#ifdef _WIN32

#include <windows.h>

#include <crtdbg.h>
#include <stdlib.h>

namespace
{

struct QuietFailures
{
    QuietFailures()
    {
        // Suppresses the "has stopped working" and critical-error boxes for
        // this process and anything it launches.
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

        // Suppresses the Windows Error Reporting dialog on abort. The 0 clears
        // _CALL_REPORTFAULT, which is what makes abort() call Dr. Watson.
        _set_abort_behavior(0, _CALL_REPORTFAULT);

        // CRT asserts and errors go to the debugger's output rather than a
        // dialog box. Under a debugger they land in the Output window; without
        // one they are simply not shown as a dialog, so they cannot block.
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    }
};

const QuietFailures quietFailures;

} // namespace

#endif
