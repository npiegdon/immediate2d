#pragma once

//
// This is broken off from the main exampleB file to keep things a
// little less visually noisy over there, but everything here is
// still just plain C/C++ code.
//
// Since the the size of the window is fixed, we know the dimensions
// of every level up front.  That means it'll be easy enough to encode
// the level in a regular string like this:
//
// const char *aLevel = "################"
//                      "#           -  #"
//                      "#  @           #"
//                      "#              #"
//                      "#             |#"
//                      "#        1     #"
//                      "#              #"
//                      "#   $          #"
//                      "#  $$$         #"
//                      "#   $       !  #"
//                      "#       ab     #"
//                      "################"
//                      "1:a#b$.";
//
//  Space : Floor
//      # : Wall
//      @ : Player
//      $ : Coin
//      ! : Exit door
//      - : Horizontal bug (initially facing east)
//      | : Vertical bug (initially facing south)
// 0 to 9 : Trigger
// A to Z : A point of interest (that starts as a wall)
// a to z : A point of interest (that starts as an empty floor space)
//
// After the 16x12 (= 192) standard tiles for the room, there is a list of
// trigger effects.  The format is: a trigger number, a colon, a point of
// interest, one of the regular tile characters from the list above that
// should replace whatever is currently there, and finally a period.
//
// Examples:
//   1:B-.   Stepping on trigger 1 creates a horizontal bug at point B.
//   3:a#.   Stepping on trigger 3 creates a wall at point a.
//
// (Points are case-sensitive, so Q and q are different points!)
//
// A trigger is allowed to have more than one effect:
//   0:a#b$c$d$.  Stepping on trigger 0 creates a wall and three coins
//                at points a, b, c, and d.
//
// To make it easier to create larger effects, you can reuse point-of-
// interest letters.  If you create a level with ten point G's, then
// "7:G$." will make ten coins when you step on trigger 7.
//
// Advanced trigger usage: a trigger can even create another trigger.
// The action lists are used as a kind of reusable template so more than
// one trigger can use it.  "3:G4" causes trigger 3 to create trigger 4 at
// point G.  There are lots of possibilities like creating cycles (4 could
// re-create 3 someplace else, etc.)
//
// To create the kinds of situations described above (like cycles), it may
// be desirable to have a level begin with a trigger on top of a point of
// interest, but our level data format doesn't let us encode that directly.
// As a workaround, trigger numbers may also be used as a point of interest
// name.  The result is a little clumsy looking ("create trigger 1 at the
// point trigger 1") but it works.  Here is an example cycle:
// 
//   "1:22..."
//   "2:11..."
// 
// Trigger 1 creates trigger 2 at point 2.  Trigger 2 creates trigger 1 at
// point 1.  (For this to be an interesting game mechanic, at least one of
// those triggers should probably also do more than just spawn the other!)
//
// NOTE: As far as using trigger numbers as points of interest goes, their
//       fixed positions are resolved during level load.  There isn't any
//       dynamic behavior (say, checking for currently existing triggers
//       that may have been spawned by another trigger in the meantime) when
//       an action list is run.  The numbers are simply treated as a wider
//       alphabet of available point names.
// 
// At this point we know how to make a level that begins with a floor, wall,
// or trigger on top of a point of interest.  Sometimes you may want one of
// the other tile types to start on a point instead.  For these last cases,
// we introduce a special trigger template that is always run when the level
// is loaded.  It uses the "~" symbol.  To have an exit door on point J at
// the start of the level you could include "~:J!." in the trigger template
// section of the level string.  This is less convenient than the floor,
// wall, and trigger cases where they can be encoded directly in the "floor
// plan" part of the level string, but now we have a way to have a level
// start with any type of tile on a point of interest.
//

constexpr const char *LevelList[] =
{
    "                "
    "                "
    "     ######     "
    "     #@ #$#     "
    "     ## # #     "
    "     #$   #     "
    "     #### #     "
    "     #!## #     "
    "     #    #     "
    "     ######     "
    "                "
    "                ",

    "                "
    "                "
    "    ########    "
    "    #@   #!#    "
    "    #### A #    "
    "    #    ###    "
    "    # ## #$#    "
    "    #  # # #    "
    "    # 1#   #    "
    "    ########    "
    "                "
    "                "
    "1:A .",

    "                "
    "   ##########   "
    "   # $#-   1#   "
    "   # #### ###   "
    "   #      # #   "
    "   # ## # # #   "
    "   #  #@#  |#   "
    "   ## ##### #   "
    "   #aa#!#   #   "
    "   #aa#   #$#   "
    "   ##########   "
    "                "
    "1:a$.",

    "      #####     "
    "      #  |#     "
    "      #2# #     "
    "      #1#a#     "
    "      # # #     "
    "      #@#|#     "
    "      ###|#     "
    "        #$#     "
    "        # #     "
    "        #|##    "
    "        # !#    "
    "        ####    "
    "~:2 ."
    "1:a#22."
    "2:a 11.",

    "################"
    "#   #1####   #a#"
    "# @ # ####   # #"
    "#  ## #### |## #"
    "#     ####  |  #"
    "### #$###### # #"
    "#-   |####     #"
    "# # # #### # # #"
    "#-  # ####   # #"
    "# # # #### # # #"
    "#   # ####!- # #"
    "################"
    "1:a@.",
};
