# Project Planning

# Minimum Viable Product (MVP):

* Can not be just a web app because the game can not exist just within the browser  
  * Should be an app because there needs to be a place for the controller and way to swap controls  
  * If P1 and P2 are both playing games, they will both need the app   
  * Should be actual desktop application due to this  
* Need way to dictate who can join/play  
* Need to pass controller  
* Need way to access game  
* Need interface for people to communicate  
* 

| Minimum | Extension |
| :---- | :---- |
| Public/private voice call: lyla Public/private chat rooms: lyla WebRTC screensharing/streaming the game: lyla, simon P2P hosting: lyla Desktop application: lyla Profiles Backend Database for Profiles Drivers to interface with controllers Controller input support Host control (invite, mute, kick) Join via access code? Only host needs to own the game | Video call Hybrid/centralized server architecture App customization Record last 30 seconds Keyboard/mouse input support Host swap |

Roles and what people would like to do:  
Simon: network stuff, not opposed to having second network person  
Michael: backend databases and networking  
Lyla: if needed, low level ish tasks in C but also interested in learning frontend but would need to look into libraries and how it’s done. could also do general planning for graphic design of frontend. skills are very good for this, especially is early 2000s theme. could help with controller stuff. could help with voice over IP  
Sam: controller inputs, screensharing webrtc  
Joshua: testing, might consider networking

Protocols:  
ICE, XMPP, STUN, TURN, Single Ext

**Notes (Section 2):**

* Would like to model off of mid 2000s internet  
  * More self-hosted and p2p to make networks temporarily that go down if you don’t have the server  
* Could have self-hosted servers  
* Would have to store profiles on given person’s host machine since we wouldn’t have central server  
  * Password auth and 2FA: how would we do this if not centralized? Would have to give some of your info for this  
  * Could also be like anon message board where you put in a nickname and don’t necessarily need an account  
* Think about our audience: groups of friends or finding new people?  
  * Could be both  
* Servers host/store chats  
* If server goes down, chats go down with them  
* Clients all connect to the servers  
* Saves money since we don’t need to host the servers  
* Monetization not rn bleh  
* Simon and Michael talking about Palantir  
  * We could get bought out by Google if our product is so good  
* Could fork our own product and get sued  
* Instead of hosting message rooms ourselves, it’s up to the server which is just another client to do it and you can connect to them in a distributed architecture  
* Hypothetical idea of hybrid system where instead of p2p distributed it could be p2p but when the server peer goes down then we go to a backup server that we host so that we offload some of the bandwidth we need infrastructure wise  
  * Best of both worlds and need architecture behind it  
* Anyone can host their own matrix server and anyone can connect to it but anyone can connect to any server  
  * Mastodon is very similar \- decent model  
    * Have own central server and anyone can connect to any server  
* Are we leaning P2p or more hybrid?  
  * Most agree p2p for minimum and hybrid for extension  
  * Electricity cost for us for hybrid and more money spent for us is a no no right now  
* Streaming of game in desktop app wouldn’t be in virtual machine or web  
  * Not doing mobile app  
  * DESKTOP ONLY  
  * WebRTC is video streaming protocol  
    * Video game streaming  
* GitHub pages site  
* Could have a highly customizable extension like old MySpace pages or Youtube  
* AI agent to help less technical people customize their site  
  * Would need to market to less technical people  
  * Probably not an extension  
* What OS do we want to target?  
  * Currently Linux, also want Windows as an extension though  
  * Might shoot ourselves in the foot since we would have to recompile for every OS  
  * Could automate some of it with GitHub action runners  
  * GitHub page and could make an AUR package so that people could build it themselves  
  * Could get it on Pacman  
  * 95% of gamers are on Windows, 3% Steam Deck, 2% Other Linux  
    * Honestly kind of crazy  
  * All of us use Linux though, some have Windows (or even MacOS)  
  * Joshua asks if we can have our OS issues taken care of by using Python  
    * Simon says that we’re currently considering Python but may switch depending on all our protocols so we will have to see  
  * Windows has benefits on market share since there are so many people using it  
  * Do we want to target more technical people or the majority of people?  
    * Michael believes we should target the majority of people  
    * Luke says that there are still developers on Windows  
  * We should support TempleOS  
  * Building network stack in Holy C  
    * Please don’t do this  
  * Should have one of the secret jingles (terry songs) along with the mimic song  
  * Windows primary  
    * Luke says “or cross compilation” and he can look into it  
* Could have two input modes  
  * Keyboard  
  * Supported controllers  
* Adding friends and making servers should be seamless  
* How do we make accounts?  
  * Wouldn’t need passwords, just OTP/OTC from email  
    * If email gets hacked, you’re screwed  
  * How do we save user profile configs if we were to have no central accounts and just SSOs?  
  * Could authenticate with GitHub or Google or something else so we don’t have to authenticate ourselves  
  * If you have a server that doesn’t get backed up and that you can’t get into then what’s the point?  
* Looking into the bootstrapping of networks like matrix and mastodon and how they do it  
* Stack-like: older things get overwritten and space is reallocated for new messages  
  * Not completely volatile  
  * Kind of like 4Chan   
  * Combine best aspects of 4Chan, Discord, and Mastodon  
    * what are we even doing atp  
    * it is targeted at gamers and developers i guess  
    * this is going to be so problematic  
* Inputs can vary between OS  
  * What you’re using would determine how inputs need to be sent

**Notes (Section 1 (don’t ask why 1 is below 2)):**

* There’s a lot of libraries and apps online that do what we want  
  * These could help us achieve MVP and give us time to focus on the finer details and core parts of our project  
  * Would mean that we don’t need to build from the ground up and would allow us to increase the scope of the project  
* Can have 2/3 of cheap, fast, or good (Project Management Triangle)  
* Things can get more complicated than you can account for, so we need to be cautious of what we are implementing and scope-creep  
* Could have centralized server or p2p, but what happens if network blocks p2p?  
  * Places that block p2p generally try to block torrenting apps/services  
  * P2p allowed on most networks under different protocols for networking  
  * Ice protocol- stun and turn  
    * Stun is more modern for p2p but also gets blocked more  
    * Turn is more reliable and can be used for p2p  
      * Can be used as backup if stun failed  
    * XMPP and jingle  
      * Jingle seems to be exactly suited for what we want  
      * Can add p2p session control with voice over IP, file transfer, and a bunch of stuff we want  
      * Not sure how it can work for the controller, but worth it for the discord clone side of it  
    * Look into Ice protocol and Jingle for networking for our project  
      * Can try by building mini networks with each network for hybrid and direct p2p by using the protocol suites and see which one performs best, interfaces best, etc  
      * Hardware has to be decided, but can use PC, RPi  
  * Think about requirements for a lot of things  
    * Timely, need short latency for controller  
    * What does network HAVE to do?  
    * What does app HAVE to do?  
    * Find things around what we need for the reqs  
  * Protocols are predeveloped so we need less development time for that  
  * Simon can look into protocols  
    * Believes the discord clone portion can work through ice and jingle  
  * Simon is network guy  
  * If we have greatest network, we still need to figure out what to actually do with it  
* How do we want to communicate with what we find?  
  * Constant communications, trade studies, project proposals?  
  * Informal process can work though  
  * Distributed architecture in case someone is sick, doesn’t respond, etc  
  * Would you prefer to be in integration hell or preplanning phase?  
    * Obviously not integration hell, but make sure to preplan and put our ideas in there so everyone can understand the reasoning  
      * Can throw in a doc or whatever format works best	  
* Let’s make a pizza and slice it mmmmm i could really go for pizza right now  
  * maybe veggie or sausage i love veggie pizza  
* Down for Python instead of C++