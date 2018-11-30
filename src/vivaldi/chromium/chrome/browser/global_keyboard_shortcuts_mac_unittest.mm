// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <AppKit/NSEvent.h>
#include <Carbon/Carbon.h>
#include <stddef.h>

#include "chrome/browser/global_keyboard_shortcuts_mac.h"

#include "base/macros.h"
#include "base/stl_util.h"
#include "chrome/app/chrome_command_ids.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/ui_features.h"

namespace {

int CommandForKeys(bool command_key,
                   bool shift_key,
                   bool cntrl_key,
                   bool opt_key,
                   int vkey_code) {
  NSUInteger modifierFlags = 0;
  if (command_key)
    modifierFlags |= NSCommandKeyMask;
  if (shift_key)
    modifierFlags |= NSShiftKeyMask;
  if (cntrl_key)
    modifierFlags |= NSControlKeyMask;
  if (opt_key)
    modifierFlags |= NSAlternateKeyMask;

  // Use a two-length string as a dummy character.
  NSString* chars = @"ab";

  NSEvent* event = [NSEvent keyEventWithType:NSKeyDown
                                    location:NSZeroPoint
                               modifierFlags:modifierFlags
                                   timestamp:0.0
                                windowNumber:0
                                     context:nil
                                  characters:chars
                 charactersIgnoringModifiers:chars
                                   isARepeat:NO
                                     keyCode:vkey_code];

  return CommandForKeyEvent(event).chrome_command;
}

}  // namespace

TEST(GlobalKeyboardShortcuts, BasicFunctionality) {
  // Test that an invalid shortcut translates into an invalid command id.
  EXPECT_EQ(-1, CommandForKeys(false, false, false, false, 0));

  // Check that all known keyboard shortcuts return valid results.
  for (const auto& shortcut : GetShortcutsNotPresentInMainMenu()) {
    int cmd_num = CommandForKeys(shortcut.command_key, shortcut.shift_key,
                                 shortcut.cntrl_key, shortcut.opt_key,
                                 shortcut.vkey_code);
    EXPECT_EQ(cmd_num, shortcut.chrome_command);
  }
  // Test that switching tabs triggers off keycodes and not characters (visible
  // with the Italian keyboard layout).
  EXPECT_EQ(IDC_SELECT_TAB_0,
            CommandForKeys(true, false, false, false, kVK_ANSI_1));
}

TEST(GlobalKeyboardShortcuts, KeypadNumberKeysMatch) {
  // Test that the shortcuts that are generated by keypad number keys match the
  // equivalent keys.
  static const struct {
    int keycode;
    int keypad_keycode;
  } equivalents[] = {
    {kVK_ANSI_0, kVK_ANSI_Keypad0},
    {kVK_ANSI_1, kVK_ANSI_Keypad1},
    {kVK_ANSI_2, kVK_ANSI_Keypad2},
    {kVK_ANSI_3, kVK_ANSI_Keypad3},
    {kVK_ANSI_4, kVK_ANSI_Keypad4},
    {kVK_ANSI_5, kVK_ANSI_Keypad5},
    {kVK_ANSI_6, kVK_ANSI_Keypad6},
    {kVK_ANSI_7, kVK_ANSI_Keypad7},
    {kVK_ANSI_8, kVK_ANSI_Keypad8},
    {kVK_ANSI_9, kVK_ANSI_Keypad9},
  };

  for (unsigned int i = 0; i < base::size(equivalents); ++i) {
    for (int command = 0; command <= 1; ++command) {
      for (int shift = 0; shift <= 1; ++shift) {
        for (int control = 0; control <= 1; ++control) {
          for (int option = 0; option <= 1; ++option) {
            EXPECT_EQ(CommandForKeys(command, shift, control, option,
                                     equivalents[i].keycode),
                      CommandForKeys(command, shift, control, option,
                                     equivalents[i].keypad_keycode));
            EXPECT_EQ(CommandForKeys(command, shift, control, option,
                                     equivalents[i].keycode),
                      CommandForKeys(command, shift, control, option,
                                     equivalents[i].keypad_keycode));
          }
        }
      }
    }
  }
}