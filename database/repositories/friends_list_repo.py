from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User
from database.models.friends_list import FriendsList
from datetime import datetime, timezone


class FriendsListRepo:
    # Create -----------------------------------------------------------------------
    def add_friend(self, session: Session, newFriendPublicKey: bytes, direction: str = "OUTGOING") -> FriendsList:
        query = select(User)
        local_user = session.scalar(query)

        if local_user is None:
            raise ValueError("ERROR: add_friend() FAILED, local user does not exist")

        if direction not in ("OUTGOING", "INCOMING"):
            raise ValueError("ERROR: add_friend() FAILED, request must be OUTGOING or INCOMING")

        current_time = datetime.now(timezone.utc).isoformat()

        newFriend = FriendsList(owner_user_id = local_user.user_id, friend_public_key = newFriendPublicKey, created_at = current_time, accepted_at = None, status = "PENDING", direction = direction)

        session.add(newFriend)
        return newFriend
    

    # Read -----------------------------------------------------------------------

    def get_friendsList(self, session: Session) -> list[FriendsList]:
        query = select(FriendsList).where(FriendsList.status == "ACCEPTED")
        return list(session.scalars(query).all())
    

    def get_createdAt(self, session: Session, friendPublicKey: bytes) -> FriendsList:

        query = select(FriendsList).where(FriendsList.friend_public_key == friendPublicKey)
        time_created = session.scalar(query)
        return time_created.created_at
    


    def get_status(self, session: Session, friendPublicKey: bytes) -> FriendsList:

        query = select(FriendsList).where(FriendsList.friend_public_key == friendPublicKey)
        current_status = session.scalar(query)

        if current_status is None:
            raise ValueError("ERROR: get_status() FAILED, friend public key does not exist")


        return current_status.status
    

    def get_acceptedAt(self, session: Session, friendPublicKey: bytes) -> FriendsList:

        query = select(FriendsList).where(FriendsList.friend_public_key == friendPublicKey)
        time_accepted = session.scalar(query)

        if time_accepted is None:
            raise ValueError("ERROR: get_acceptedAt() FAILED, friend public key does not exist")



        return time_accepted.accepted_at
    
    def get_pending_request(self, session: Session) -> list[FriendsList] | None:
        
        query = select(FriendsList).where(FriendsList.direction == "INCOMING", FriendsList.status == "PENDING")
        pending_requests = session.scalars(query)

        if pending_requests is None:
            raise ValueError("ERROR: get_pending_request() FAILED, friend public key does not exist")


        return list(session.scalars(query).all())



    # Update -----------------------------------------------------------------------

    def update_status(self, session: Session, friendPublicKey: bytes, newStatus: str) -> FriendsList | None:

        query = select(FriendsList).where(FriendsList.friend_public_key == friendPublicKey)
        peer_friend_request = session.scalar(query)

    
        if peer_friend_request is None:
            raise ValueError("ERROR: update_acceptedAt() FAILED, friend public key does not exist")
        
        if newStatus not in ("PENDING", "ACCEPTED", "REJECTED"):
            raise ValueError("ERROR: update_status() FAILED, invalid status. Status must be: PENDING, ACCEPTED, or REJECTED")
        

        peer_friend_request.status = newStatus


        if newStatus == "ACCEPTED":
            if peer_friend_request.accepted_at is None:
                peer_friend_request.accepted_at = datetime.now(timezone.utc).isoformat()
        else:
            peer_friend_request.accepted_at = None
        

        session.commit()

        return peer_friend_request
    

    def update_acceptedAt(self, session: Session, friendPublicKey: bytes) -> FriendsList | None:

        query = select(FriendsList).where(FriendsList.friend_public_key == friendPublicKey)
        accepted_time = session.scalar(query)

        if accepted_time is None:
            raise ValueError("ERROR: update_acceptedAt() FAILED, friend public key does not exist")

        if accepted_time.status != "ACCEPTED":
            raise ValueError("ERROR: update_accpetedAt() FAILED, request must be ACCEPTED in order to update accepted time")
        
        accepted_time.accepted_at = datetime.now(timezone.utc).isoformat()
        session.commit()

        return accepted_time
    

    # Delete -----------------------------------------------------------------------

    def delete_friend(self, session: Session, friendPublicKey: bytes) -> bool:
        query = select(FriendsList).where(FriendsList.friend_public_key == friendPublicKey)
        friend_to_remove = session.scalar(query)

        if friend_to_remove is None:
            return False
        
        session.delete(friend_to_remove)
        session.commit()

        return True


