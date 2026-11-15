from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User

class UserRepo:
    # Create

    def user_create(self, session: Session, newFName: str, newLName: str) -> User:
        newUser = User(fname = newFName, lname = newLName)
        session.add(newUser)
        return new_user
    

    # Read

    def user_get_id(self, session: Session, user_id: str) -> User:
        return session.get(User, user_id)



    # Update

    def user_update_id(self, session: Session, user_id: str, new_user_id: str) -> User | None:
        user = session.get(User, user_id)

        user.user_id = new_user_id
        session.commit()
        return user
    

    def user_delete_id(self, session: Session, user_id: str) -> bool:
        user = session.get(User, user_id)

        if user is None:
            return False

        session.delete(user)
        session.commit()
        return True
        
