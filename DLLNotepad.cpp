#include <Windows.h>
#include <WinUser.h>
#include <vector>
#include <string>

using namespace std;

HWND hMainWindow;
HWND hTextBox;
HWND hRichEdit;
WNDPROC origWndProc;

struct EnumWindowsCallbackArgs
{
    EnumWindowsCallbackArgs(DWORD p) : 
      pid(p)
    {

    }

    const DWORD pid;
    vector<HWND> handles;
};

void showError(LPCSTR error)
{
  MessageBoxA(NULL, error, "Error", MB_ICONERROR | MB_OK);
}

BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
  EnumWindowsCallbackArgs *args = (EnumWindowsCallbackArgs *)lParam;

  DWORD windowPID;

  GetWindowThreadProcessId(hWnd, &windowPID);

  if (windowPID == args->pid)
  {
    args->handles.push_back(hWnd);
  }

  return TRUE;
}

std::vector<HWND> getToplevelWindows()
{
  EnumWindowsCallbackArgs args(GetCurrentProcessId());

  if (!EnumWindows(&EnumWindowsCallback, (LPARAM)&args))
  {
    return std::vector<HWND>();
  }

  return args.handles;
}

BOOL findHandles()
{
  vector<HWND> topWindows = getToplevelWindows();

  if (topWindows.size() == 0)
  {
    showError("Could not enumerate process' top windows");

    return FALSE;
  }

  hMainWindow = topWindows.at(0);
  hTextBox = FindWindowExA(hMainWindow, NULL, "NotepadTextBox", NULL);

  if (hTextBox == NULL)
  {
    showError("Could not find NotepadTextBox");

    return FALSE;
  }

  hRichEdit = FindWindowExA(hTextBox, NULL, "RichEditD2DPT", NULL);

  if (hRichEdit == NULL)
  {
    showError("Could not find RichEditD2DPT");

    return FALSE;
  }

  if (!SetWindowTextA(hRichEdit, "Injected!"))
  {
    showError("Could not set RichEditD2DPT text");

    return FALSE;
  }

  return TRUE;
}

DWORD WINAPI MainThread(LPVOID param)
{
  MessageBoxA(NULL, "Injected!", "DLL thread", MB_ICONINFORMATION | MB_OK);

  if (!findHandles())
  {
    return 0;
  }

  char *field = strdup(".--------------------------------.\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "|                                |\r\n" \
                "`--------------------------------'");

  DWORD current_time = GetTickCount();
  DWORD last_time = current_time;
  DWORD ellapsed = 0;
  DWORD frame_time = 250;

  int x = 16;
  int y = 8;
  int dir = 2;

  while (true)
  {
    current_time = GetTickCount();
    ellapsed += (current_time - last_time);
    last_time = current_time;

    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
      break;
    }
    else if (dir != 1 && dir != 3)
    {
      if (GetAsyncKeyState(VK_UP) & 0x8000)
      {
        dir = 1;
      }
      else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
      {
        dir = 3;
      }
    }
    else if (dir != 0 && dir != 2)
    {
      if (GetAsyncKeyState(VK_LEFT) & 0x8000)
      {
        dir = 0;
      }
      else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
      {
        dir = 2;
      }
    }

    if (ellapsed >= frame_time)
    {
      field[y * 36 + x + 1] = ' ';

      if (dir == 0)
      {
        --x;
      }
      else if (dir == 1)
      {
        --y;
      }
      else if (dir == 2)
      {
        ++x;
      }
      else if (dir == 3)
      {
        ++y;
      }

      if (field[y * 36 + x + 1] != ' ')
      {
        break;
      }

      field[y * 36 + x + 1] = '*';

      SetWindowTextA(hRichEdit, field);

      ellapsed -= frame_time;
    }

    Sleep(16);
  }

  free(field);

  return 0;
}

LRESULT CALLBACK hookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_KEYDOWN)
  {
    if (wParam == VK_RETURN && (lParam & 0b00000000000000000000000000000000) == 0)
    {
      showError("ENTER");

      return 0;
    }
  }

  return CallWindowProc(origWndProc, hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI MainThread2(LPVOID param)
{
  if (!findHandles())
  {
    return 0;
  }

  origWndProc = (WNDPROC)SetWindowLongPtr(hRichEdit, GWLP_WNDPROC, (LONG_PTR)&hookWndProc);

  return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
  {
    CreateThread(0, 0, MainThread2, hModule, 0, 0);
  }

  return TRUE;
}
