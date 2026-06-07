# Pinball: Space Wars

Pinball: Space Wars is a modern recreation of the wildly popular pinball game "3D Pinball: Space Cadet" which shipped on Windows systems in the 90s and early 2000s. Pinball: Space Wars will be a 2D game with pixel art that will be rendered in multiple layers to give the impression of 3D (parallax effect). The game will be the first game, created in the Sandbox Engine, which will integrate with a physics system in order to move the pinball.

## Game Play

- Player will have 3 lives
- Ball can be launched with full power or half power.
  - A ball launched with half power will allow the ball to enter the play area from a different location.
  - A ball launched with full power allows the ball to enter the play area at the very top of the area.
- Player has a left and right bumper which can be held into position and/or used to push the ball back up the play area.
- Various pillars and walls will be placed in static positions along the play area and when the ball hits will emit a inverted force effectively pushing the ball around.

## Physics System

- Box2D will be leveraged as it is an industry standard for 2D physics.
-
