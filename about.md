
# Practice Checkpoint Permanence (PCP)

This mod allows to quickly create <cs>persistent checkpoints</c> in <cg>practice mode</c> that you can then load at any point in the future, even across sessions.

This is very useful and convenient for **practicing sections of <cr>hard levels</c>** (effectively replaces copyables with Start Pos), and it **works on Platformer mode** too (the checkpoints store almost all relevant data).

### Usage
To use this mod you can open a level in practice mode and press the mod's keybinds (see *default* keybinds below) to *create a <cs>persistent checkpoint</c>*, then switch to the *next* or *previous checkpoint* to load it. If you want to <co>delete a checkpoint</c> then switch to that checkpoint and then press the keybind.

### Save Layers
This mod also provides "Save Layers" (Save slots) with separate checkpoints, which can be **useful for levels with multiple routes**. Layers are *created and deleted <cp>on demand</c>*, so to create one *switch to the last layer and then one more and create a <cs>persistent checkpoint</c>*; to delete one just remove all it's checkpoints.

### Checkpoint Manager
On the pause menu there's a new button that opens the <cg>persistent checkpoint list</c>, from which you can manage your saved checkpoints and layers.

### Save Manager
If you use this mod a lot you may want to <cp>delete old save files to **save space**</c>, especially if you play a lot of heavy levels. Go to the *saved levels menu* and click / touch the new button on the bottom left, it'll open a menu where you'll be able to *see some save stats* such as file size and <cr>delete saves easily</c> without having to enter each level.

## Default Keybinds

 - **Add Checkpoint**: Alt + Z
 - **Remove Checkpoint**: Alt + X
 - **Previous Checkpoint**: Alt + Q
 - **Next Checkpoint**: Alt + E
 - **Previous Layer**: Alt + Shift + Q
 - **Next Layer**: Alt + Shift + E

## Additional Notes

 - You can change the position of the switcher menu in the <cg>Practice Options menu</c>.

 - There's <cp>no limit</c> of how many <cp>checkpoints or layers</c> you can have.

 - You can press a checkpoint icon in the checkpoint list to switch to it.

 - If for any reason your save corrupts or something goes wrong or you just wanna get rid of some data you can open a level in normal mode, and press the mod's button on the pause menu to <cr>delete all persistent checkpoints from that level</c>. (Has a confirmation dialog, don't worry about missclicks)

 - The save files <cr>aren't compatible across platforms</c> (Windows, Android, etc.), when the mod finds an incompatible save file it will be ignored and overwritten on next save.

 - Saved checkpoints in <cr>editor levels are very volatile</c>, **any change to the level will invalidate it's saves**. In general it is not recommended to use this mod for editor levels.

 - Serialization provided by [PersistenceAPI](https://github.com/0x5abe/PersistenceAPI)
