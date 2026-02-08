## Unreleased
- Migrate to Geode v5, sans dependencies

## 1.3.3 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.3.3)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.3.3))
- Make the last placed persistent checkpoint be "ghost" active until switching to another checkpoint or placing a regular checkpoint \[[Issue #20](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/20)\]
- Added an option to change the position of the practice buttons to above and below the vanilla ones \[[Issue #16](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/16)\]
- The "Reset Attempts" now also resets the attempts when restarting from the last checkpoint in the Level End Screen \[[Issue #18](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/18)\]
- Added an option to change the opacity of the active checkpoint \[[Issue #19](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/19)\]
- Switching between checkpoints no longer incements the attempt counter \[[Issue #21](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/21)\]

## 1.3.2 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.3.2)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.3.2))
- You can now force load checkpoints in the Checkpoint Manager if there's an error \[[Issue #13](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/13)\]
- The attempts are no longer saved in the checkpoints, now the attempt counter is reset to one (configurable) \[[Issue #14](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/14)\]

## 1.3.1 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.3.1)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.3.1))
- Fixed the switcher not using the opacity settings if the active and inactive values were the same \[[Issue #12](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/12)\]

## 1.3.0 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.3.0)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.3.0))
- Better Platform & Practice Config UI compatibility \[[PR #9](https://github.com/Kevadroz/PracticeCheckpointPermanence/pull/9), by [Naxrin](https://github.com/Naxrin), [requested here](https://github.com/Naxrin/Better-PUI/issues/2)\]
- Added a setting for decimal precision for percentages
- Added an option to disable auto checkpoints when a persistent checkpoint is active (enabled by default)
- Fixed crash on playing an editor level on some setups \[[Issue #8](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/8)\]
- Fix Checkpoint Manager button not updating the menu layout in the pause menu
- Fix Mod Previews
- Improved memory usage a bit

## 1.2.3 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.2.3)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.2.3))
- Added support for Mod Previews
- Fix crash with Platformer Saves \[[Issue #3](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/3)\]
- Fixed compat with mods that start levels in practice mode

## 1.2.2 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.2.2)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.2.2))
- Add Mod tags
- Replace `path::string()` with `string::pathToString()`

## 1.2.1 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.2.1)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.2.1))
- Fix crash on online levels

## 1.2.0 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.2.0)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.2.0))
- Added Mac and iOS support \[[PR #2](https://github.com/Kevadroz/PracticeCheckpointPermanence/pull/2) by [hiimjasmine00](https://github.com/hiimjasmine00) and [RayDeeUx
](https://github.com/RayDeeUx)\]
- Added Persistent Checkpoints to the Progress bar
- When a save cannot be loaded the switcher will turn red with a short message and the pcp manager will now have a description of the problem
- Fixed new checkpoints not getting inserted in the correct position on classic mode on some conditions

## 1.1.0 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.1.0)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.1.0))
- Added Android support
- Added scale options for the switcher in the Practice Options menu
- Added opacity options for the switcher \[[Issue #1](https://github.com/Kevadroz/PracticeCheckpointPermanence/issues/1)\]
- Fix practice buttons not following the opacity setting properly
- Fixed a potential crash with mods that start levels in practice mode

## 1.0.2 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.0.2)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.0.2))
Fix Practice UI Config dragging the layer instead of the switcher

## 1.0.1 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.0.1)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.0.1))
- Removed Android from the supported versions

## 1.0.0 ([Release](https://github.com/Kevadroz/PracticeCheckpointPermanence/releases/tag/v1.0.0)) ([Source](https://github.com/Kevadroz/PracticeCheckpointPermanence/tree/v1.0.0))
- Released!!! 🥳 🥳 🥳
