from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User
from database.models.user_settings import UserSettings
from database.repositories.user_settings_repo import UserSettingsRepo

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
            return
        

        local_user.user_id = new_user_id
        session.commit()

        return local_user

    def update_local_publicKey(self, session: Session, newPublicKey: bytes) -> User | None:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            return
        

        local_user.public_key = newPublicKey
        session.commit()

        return local_user

    def update_local_passwordHash(self, session: Session, newPasswordHash: str) -> User | None:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            return
        

        local_user.password_hash = newPasswordHash
        session.commit()

        return local_user
    

    def update_local_firstName(self, session: Session, newFirstName: str) -> User | None:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            return
        

        local_user.first_name = newFirstName
        session.commit()

        return local_user
    
    def update_local_lastName(self, session: Session, newLastName: str) -> User:
        query = select(User)
        local_user = session.scalar(query)
        
        if local_user is None:
            return
        

        local_user.last_name = newLastName
        session.commit()

        return local_user
    

    # Delete -----------------------------------------------------------------------

    def delete_user(self, session: Session) -> bool:

        query_for_user = select(User)
        query_for_settings = select(UserSettings)


        local_user = session.scalars(query_for_user).first()
        local_settings = session.scalars(query_for_settings).first()



        if local_user is None or local_settings is None:
            return False
        

        # Remove child tables first
        session.delete(local_settings)
        session.flush()

        # Remove the user itself
        session.delete(local_user)

        session.commit()
        return True
    

    
    
