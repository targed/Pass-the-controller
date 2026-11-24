from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User
from database.models.user_settings import UserSettings
from database.repositories.user_settings_repo import UserSettingsRepo
from database.models.known_peer import KnownPeer
from database.models.host_ban import HostBan

# Later optimizations: 
# - user_create should call the functions for creating the key and password hash as a user should have those upon creation. More convenient if whole thing happens in one go
# - Some functions are logically identical (such as create and update firstname)

class UserRepo:

    # Create -----------------------------------------------------------------------

    def create_user(self, session: Session, newFName: str, newLName: str) -> User:
        newUser = User(first_name = newFName, last_name = newLName, user_id = "user_id", public_key = b"public_key", password_hash = "password") #Default values
        session.add(newUser)

        session.flush()

        user_repo_settings_instance = UserSettingsRepo()
        user_repo_settings_instance.create_userSettings(session) # Assign User it's corresponding settings

        return newUser



    # Read -----------------------------------------------------------------------

    def get_local_id(self, session: Session) -> User | None:
        query = select(User)
        return session.scalar(query)

    def get_local_publicKey(self, session: Session) -> User | None:
       query = select(User.public_key)
       return session.scalar(query)
    
    def get_local_passwordHash(self, session: Session) -> User | None:
        query = select(User.password_hash)
        return session.scalar(query)

    def get_local_firstName(self, session: Session) -> User | None:
        query = select(User.first_name)
        return session.scalar(query)

    def get_local_lastName(self, session: Session) -> User | None:
        query = select(User.last_name)
        return session.scalar(query)



    # Update -----------------------------------------------------------------------

    def update_local_id(self, session: Session, new_user_id: str) -> User | None:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            raise ValueError("ERROR: update_local_id() FAILED, user does not exist")
        

        local_user.user_id = new_user_id
        session.commit()

        return local_user

    def update_local_publicKey(self, session: Session, newPublicKey: bytes) -> User | None:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            raise ValueError("ERROR: update_local_publicKey() FAILED, user does not exist")
        

        local_user.public_key = newPublicKey
        session.commit()

        return local_user

    def update_local_passwordHash(self, session: Session, newPasswordHash: str) -> User | None:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            raise ValueError("ERROR: update_local_passwordHash() FAILED, user does not exist")
        

        local_user.password_hash = newPasswordHash
        session.commit()

        return local_user
    

    def update_local_firstName(self, session: Session, newFirstName: str) -> User | None:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            raise ValueError("ERROR: update_local_firstName() FAILED, user does not exist")
        

        local_user.first_name = newFirstName
        session.commit()

        return local_user
    
    def update_local_lastName(self, session: Session, newLastName: str) -> User:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            raise ValueError("ERROR: update_local_lastName() FAILED, user does not exist")
        

        local_user.last_name = newLastName
        session.commit()

        return local_user
    

    # Delete -----------------------------------------------------------------------

    #  Not only deletes user but it's child tables as well
    def delete_user(self, session: Session) -> bool:

        query_for_user = select(User)
        query_for_settings = select(UserSettings)
        
        local_user = session.scalars(query_for_user).first()

        if local_user is None:
            raise ValueError("ERROR: update_local_id() FAILED, user does not exist")


        local_settings = session.scalars(query_for_settings).first()

        # Anytime a table has multiple users, we must collect all entries and use loops to delete each tuple one-by-one
        local_knownPeers = session.scalars(select(KnownPeer).where(KnownPeer.owner_user_id == local_user.user_id)).all()
        local_bans = session.scalars(select(HostBan)).all()

        
        
        # Remove child tables first
        if local_settings is not None:
            session.delete(local_settings)

        # Remove all user's from ban list
        for peer in local_bans:
            session.delete(peer)

        # Find the known peer that matches the user on track to being deleted
        for peer in local_knownPeers:
            session.delete(peer)

        session.flush()

        # Remove the user itself
        session.delete(local_user)

        session.commit()
        return True
    

    
    
