**Pass the Controller Project Proposal**

**Introduction**  
Our team is assigned to create a web application called Pass the Controller. In this project, we will be creating a Discord clone that has the added functionality of a virtual controller that you can “pass” to other users. The main objective of Pass the Controller is to replicate the experience of playing a video game with friends in the same room and taking turns by passing the controller, but through an online social gaming application. Pass the Controller will allow users to host gaming sessions that others can join remotely. During a session, users can seamlessly “pass the controller” to other members, watch gameplay in real-time, and communicate through either voice chat or text. The host maintains control over the session and determines who is allowed to join and who currently has input control of the game. 

**Objectives**

1. *Login Page*  
- We will need to create a log in page for users where they can create an account or  
  sign in to an existing one. Users will need to provide a username and password.  
    
2. *Interface for Messaging*  
- We will need to create a basic interface for messaging so that users can interact with each other outside of calling for video games.


3. *Interface for Voice Calling \[Server/Private\]*

		\-We will need an interface for both group calls and individual direct messaging  
 calls.

4. *Virtual Controller to “Pass” to Other Users*  
- We will need an interface to facilitate the “passing” of the controller remotely with needed security protocols for controller “passing” and private communications so client devices are secure from malicious users.

5. *Interface for Room/Server*  
- We will need to create an interface for the room or server that users will be in so that they can interact with other users and “pass” the controller. This could be implemented as a chat room that users are specifically invited to, a large overarching server that people can join whenever, or some small room that exists for the duration that the game is played. Specific decisions will be made later.

6. *Host Control Interface for Controller Passing and Basic Configuration*  
- We will need to create an interface to allow a given end user to “pass” the controller over to another user for remote play. Further we will need to create an interface with set rules for controller passing. Security options, time limits, applications allowed to be remotely controlled, etc.

7. *Settings*  
- We will need to create various user settings so that the user can decide on their own Pass the Controller experience. There will be application settings such as deciding who can join/use the virtual controller or how many people can be in the room/use the controller. There may also be some user settings to allow users to change their username or add a profile picture.

8. *P2P Hosting \[Possible concerns with hosting in apartment complexes\]*  
- We need to figure out if we will host this on a central server or utilize Peer to Peer communication, but we will implement some form of communication.  
    
  * **Ideas for Possible Implementation**  
1. *Basic Image/GIF/File Sending*  
- If we have time, we may add in functionality to allow users to attach files such as images, GIFs, or small game files.  
- There may also be a possible tool for auto-suggesting/taking screenshots and sending them to the chat.

2. *User Profiles*  
- We may add in additional functionality to customize user profiles, such as profile banners, statuses, or animated profile pictures.

3. *2-Factor Authentication Login*  
- We may allow users to authenticate with a one time email/text code or an authentication app.


                  *4\.   Retroactively Record 30-Second Clip*

- We may add a feature that allows users to press a button that will capture a short video that includes frames of the past 30 seconds in case there is a clip the user wants to save.

* **Roles and Responsibilities**  
  * Project Manager / Scrum Master: Luke Kaiser  
  * Product Owner / Requirements Lead: Michael Allen  
  * System Architect / Design Lead: Sam Hajdukiewicz  
  * Frontend Developer / UI Lead: Lyla Graham  
  * Backend Developer / Database Lead: Sam Hajdukiewicz  
  * QA / Testing Lead: Joshua Dobbs  
  * Documentation / Deployment Lead: Simon Leeker

* **Actors/Users**  
  * Team Pass the Controller: Sam Hajdukiewicz, Luke Kaiser, Simon Leeker, Lyla Graham, Michael Allen, Joshua Dobbs  
  * Project Manager/Client: Luke Kaiser  
  * Users: Gamers, developers, students

* **Project Constraint**  
  * One of the biggest issues we will face for this project is a limited time window. We will be working on this project for two semesters, so we have a due date of 8 months from now. Due to the large scope of this project, we may be constrained on time, especially accounting for debugging issues.  
      
* **Team Information**  
  * Sam Hajdukiewicz \- Knowledgeable in C++, Python, some HTML, some Javascript, and databases.  
  * Luke Kaiser \- Versed in working with, training, implementing, and testing various AI models. Proficient in C, C++, Bash, JavaScript, PowerShell, and Python. Adept at various Linux distributions, version control,  package managers, Server management, QA testing, infrastructure, containerization, and computer life cycle management.  
  * Simon Leeker \- C++, Python, Networking, Distributed Systems, 5g/6g  
  * Lyla Graham \- C++, currently learning C, Git, Linux, Vim, decent experience with Java. Worked with Typescript a few times but barely familiar with it  
  * Michael Allen \- Knowledgeable in C/C++, C\#, Python, SQL, Git, Linux, and Backend development. Learning networking and machine learning   
  * Joshua Dobbs \- Skilled with Linux, Git, networking, C, and Python


  
**Team Policies**

* **Meeting Schedule**  
  * We will meet every Wednesday at 6 PM in the Innovation Lab (or wherever the team decides to meet that day). 

* **Attendance Expectations**  
  * All team members must attend meetings unless the team votes not to meet that day, the meeting is moved to another day, or there are unforeseen circumstances (such as illness, travel, family death, etc.) or a holiday.   
  * If a team member misses 3 consecutive team meetings with no communication and/or has not responded to any Discord communications for 7 days, this will be brought up to the professor and cause restructuring or removal from the team.  
      
* **Communication Expectations**  
  * Team members shall communicate over Discord and shall respond to messages within 24-48 hours  
      
* **Task Assignment**  
  * After we have our idea solidified and a plan to complete the project, we will determine what areas our members are most experienced in. Members will be able to work on the parts they are most familiar with, and if there are any aspects of the project where people are unfamiliar with the concept, we will determine who has the most bandwidth to learn and develop that portion. We will assist each other where needed, but for the majority of our project, we will work on our own sections.  
      
* **Conflict Resolution**   
  * Any conflicts in designs or social aspects should firstly be addressed between the two conflicting individuals. If that does not resolve the issue, a larger team gathering will be required to address the situation. If that fails, this will be taken to the professor. Any conduct that immediately violates the school, social, or academic policies shall be brought up to the respective school authorities in accordance with school policies.   
  * If one has concerns, it is better to state them up front rather than to let things simmer.  
  * Any major design or social decisions shall be voted on in a ranked choice voting system. If that is not possible for the given situation, first-past-the-post shall be used.   
      
* **GitHub or Version Control Usage**  
  * All our code is stored on our [Github](https://github.com/targed/Pass-the-controller).   
  * All members are expected to contribute, version, review, and merge their code on this Github repository.   
      
* **Deadline Management**  
  * Deadlines and project milestones are derived from the course expectations and deadlines. They will dictate what we have to do and by when. However, those will not necessarily be sufficient for what we need. Thus, we will create some of our own internal deadlines in order to break these large tasks up into smaller, more manageable chunks. These deadlines are flexible but do all need to come together for the entire project and can delay other projects down the line.   
  * Thus, deadline management shall look as follows: we will send out periodic updates and reminders about specific projects and deadlines. The group will also independently verify the progress other members have made, whether it is up to expectations, and if not, mitigations will be decided on as a group. (such as redistribution or swarm tactics)  
      
* **Quality Standards**  
  * There will be comprehensive unit, integration, regression, smoke, frontend, API, and backend tests. This will ensure our code meets the requirements with multiple people developing and merging code.   
  * AI development is acceptable as long as the member knows what it does, it solves a problem, and it solves the problem efficiently. 