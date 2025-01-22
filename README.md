# Pizza Connection 2 - Trainer
Greetings to the old guard and the only right path, i.e., the old school. As a fan of classic games, I am honored to present you with a small trainer for the game Pizza Connection 2, which may also be called Fast Food Tycoon 2. This game had its global premiere on February 2 or 10, 2001, depending on the source. It hit the American market on October 9, 2001, and the Polish premiere took place on March 21, 2001. The developer was, of course, Software 2000, and the Polish distributor was CD Projekt. Kudos to all the people who worked on this project. It's been twenty-something years since we've been able to enjoy it.
## A short story
A long time ago, when CDs reigned supreme, I decided to buy a copy of the game as a fan of economic and strategy games. In a time when the Internet was not so widely available, different rules prevailed. One day, I came across the "Engage your Mind" Trainer created by Keyboard Junky & VandalJax from EliteServers.de. I thought to myself, "That's a cool convenience." As a kid, many things were simpler, and there were many things we had no idea about, especially how binary code worked. The aforementioned Trainer didn't work exactly as it should, so I decided to build the same one from scratch. The tool currently being published was created based on the GOG distribution with the binary named fastfood2.exe (formerly known as pizza2.exe) in version 1.006. The functions are identical to the original.
## Old trick
An old trick was to change the base price of the pizza before the waiter received it.
## Multiplayer
Officially, the game does not have a multiplayer mode, but in the resource files, you can find the (in German) NETZWERK and GAMELOBBY dialogs, which suggest that it may have been planned. There’s a funny story about a dialog box that has the text "A very nasty thing has happened" in the caption.
## Changing the game language
User Colek on the [Steam community forum](https://steamcommunity.com/sharedfiles/filedetails/?id=1080744152) wrote about how to change the game language, providing an example of Polishization.
## Window mode
Yes, that's true, Pizza Connection 2 can be run in windowed mode, which will make a few things easier :) User Colek on the [Steam community forum](https://steamcommunity.com/sharedfiles/filedetails/?id=1080677307) wrote instructions on how to perform such an operation. In short, you just need to use the DxWnd program from the appropriate source.
## Bugs in the game
There are bugs in the game that cause strange behavior. For example, the number of restaurants you can have on a given map is limited in terms of building and renting, but not in terms of warehouses. Once the target number of warehouses is exceeded, the game will crash. Holding the right mouse button in single restaurant viewing mode speeds up NPC movement (and game clock). It seems to me that this may be the correct speed for the virtual people, but due to a bug in the main game loop, we experience what we have, i.e., a slow-motion effect. The icing on the cake is the poorly written game logic that allows for such [excesses](https://youtu.be/G72a3XUYnAE). I recommend that everyone spend some time looking for additional errors. I assure you, there are at least a few of them.
## License
Honestly? The Beerware license fits best here :) The program was written in C++ in Visual Studio 2017, in case anyone was asking. I encourage you to compile it yourself.
## Ethical solutions
Remember that creating such tools for online games is not a fair solution.
## Test environment
I tested the binary on 64-bit Windows 7. In theory it should work on all systems.
## Postscriptum
Greetings to everyone involved in any way in this project.