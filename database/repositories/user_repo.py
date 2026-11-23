from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User

# Later optimizations: 
# - user_create should call the functions for creating the key and password hash as a user should have those upon creation. More convenient if whole thing happens in one go
# - Some functions are logically identical (such as create and update firstname)

class UserRepo:
    # Create -----------------------------------------------------------------------

    def user_create(self, session: Session, newFName: str, newLName: str) -> User:
        newUser = User(first_name = newFName, last_name = newLName)
        newUser.user_id = "user_id"
        newUser.public_key = b"public_key"
        newUser.password_hash = "password"
        session.add(newUser)
        return newUser

    def user_create_publicKey(self, session: Session, user_id: str) -> User:
        user = session.get(User, user_id)

        if user is None:
            return


        user.public_key = b"public_key" # The "b" converts the text into bytes
        return user


    def user_create_passwordHash(self, session: Session, user_id: str) -> User:
        user = session.get(User, user_id)

        if user is None:
            return

        user.password_hash = "password" # Temp value for now
        return user
    
    def user_create_firstName(self, session: Session, user_id: str, newFName: str) -> User:
        user = session.get(User, user_id)

        if user is None:
            return


        user.first_name = newFName

        return user

    def user_create_lastName(self, session: Session, user_id: str, newLName: str) -> User:

        user = session.get(User, user_id)

        if user is None:
            return

        user.last_name = newLName

        return user

    
        
    

    # Read -----------------------------------------------------------------------

    def user_get_id(self, session: Session, user_id: str) -> User:
        return session.get(User, user_id)

    def user_get_publicKey(self, session: Session, user_id: str) -> User:
        user = session.get(User, user_id)
        if user is None:
            return

        return user.public_key
    
    def user_get_passwordHash(self, session: Session, user_id: str) -> User:
        user = session.get(User, user_id)

        if user is None:
            return

        return user.password_hash

    def user_get_firstName(self, session: Session, user_id: str) -> User:
        user = session.get(User, user_id)

        if user is None:
            return

        return user.first_name

    def user_get_lastName(self, session: Session, user_id: str) -> User:
        user = session.get(User, user_id)

        if user is None:
            return

        return user.last_name



    # Update -----------------------------------------------------------------------

    def user_update_id(self, session: Session, user_id: str, new_user_id: str) -> User | None:
        user = session.get(User, user_id)

        if user is None:
            return

        user.user_id = new_user_id
        session.commit()
        return user

    def user_update_publicKey(self, session: Session, user_id: str, newKey: bytes) -> User | None:
        user = session.get(User, user_id)

        if user is None:
            return

        user.public_key = newKey
        session.commit()
        return user

    def user_update_passwordHash(self, session: Session, user_id: str, newPassword: str) -> User | None:
        user = session.get(User, user_id)

        if user is None:
            return
        

        user.password_hash = newPassword
        session.commit()
        return user
    

    def user_update_firstName(self, session: Session, user_id: str, newFName: str) -> User | None:
        user = session.get(User, user_id)

        if user is None:
            return
        

        user.first_name = newFName
        session.commit()
        return user
    
    def user_update_lastName(self, session: Session, user_id: str, newLName: str) -> User:
        user = session.get(User, user_id)

        if user is None:
            return
        

        user.last_name = newLName
        session.commit()
        return user
    

    # Delete -----------------------------------------------------------------------

    def user_delete_id(self, session: Session, user_id: str) -> bool:
        user = session.get(User, user_id)

        if user is None:
            return False

        session.delete(user)
        session.commit()
        return True

    
    
