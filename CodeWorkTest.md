Hazelight Code Work Test
========================
One of the responsibilities of gameplay programmers at Hazelight is to take an initial prototype version
of a gameplay mechanic and iterate on it. This includes fixing bugs, making improvements, adding new
functionality as requested by the design team, and taking initiative in experimenting with new ideas.

This work test is an example of such a prototype mechanic, and contains several requests for 
fixes and improvements, as well as an expansion to the mechanic.

Setup
=====
Install Unreal Engine from the Epic Games launcher, downloadable at https://www.epicgames.com/store/en-US/download

This folder contains a C++ game project based on the third person example provided with the engine.
The code test project was created in a vanilla Unreal Engine version 4.27.2.
Generate visual studio project files from the context menu of the uproject file.

The Mechanic
============
The Design team has requested a system for the player character to perform a rope swing from
specific points placed out into the game world. In collaboration with a programmer, a prototype
has been made and approved for further development.

The intent is for the player to press and hold a button (currently the 'e' key,
or the right trigger on a controller) to attach them to a specific rope swing
point with a rope, then allowing them to swing across gaps and gain velocity
from the attachment.

Now that the mechanic is approved, several aspects of it will need to be
improved first before it can be used in any actual levels that are under
development.

Requests
========

- After landing out of a rope swing, the character is rotated weirdly on the ground. Please fix this.

- Currently, the rope swing always triggers on the rope attach point that is
  closest to the player. This causes unexpected behavior in several situations.
  Fix it so a more natural method is used to determine which rope attach point to use.

- The 'rope' is currently drawn as a debug line just for development purposes. The Tech Art
  team would like to experiment with some rope rendering in shaders.
  Please provide a way to display the rope in game that tech artists could then create a material for.

  Note: There is no need to implement any rope simulation, physics, or actual art,
  just an implementation of any way to display the active rope swing will do.

- The rope swing is activated using debug key input. This will not do for actual gameplay.
  Please change it so it uses the proper unreal input system for its actions.

- The Design team would like to experiment with adding a second type of rope
  attach point. When attached to this, the player would be pulled straight
  towards it, reeling in the rope. When reaching the vicinity of the attach point,
  the player will automatically detach and launch past it with its velocity.

  Provide a prototype implementation of this type of rope attach point.

