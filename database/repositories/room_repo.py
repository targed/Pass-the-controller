from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.room import Room
from database.models.room_settings import RoomSettings
from database.repositories.room_settings_repo import RoomSettingsRepo
from uuid import uuid4

class RoomRepo:
    # Create -----------------------------------------------------------------------
    
    def create_room(self, session: Session, ownerPK: bytes, roomName: str) -> Room:

        if not isinstance(ownerPK, bytes) or not ownerPK:
            raise ValueError("ERROR: create_room() FAILED, invalid owner public key")
        

        if not isinstance(roomName, str) or not roomName.strip():
            raise ValueError("ERROR: create_room() FAILED, invalid room name")

        rid = str(uuid4())

        newRoom = Room(room_id = rid, owner_public_key = ownerPK, room_name = roomName)
        session.add(newRoom)
        session.flush()

        settings = RoomSettingsRepo()
        settings.create_roomSettings(session, rid)


        return newRoom


    # Read -----------------------------------------------------------------------


    def get_allRooms(self, session: Session) -> list[Room]:
        query = select(Room)
        return list(session.scalars(query).all())


    def get_roomByID(self, session: Session, roomID: str) -> Room | None:
        query = select(Room).where(Room.room_id == roomID)
        selected_room = session.scalar(query)

        if selected_room is None:
            return None
        
        return selected_room


    
    # Delete -----------------------------------------------------------------------

    def delete_room(self, session: Session, roomID: str) -> bool:
        room_to_delete = self.get_roomByID(session, roomID)

        query = select(RoomSettings).where(RoomSettings.room_id == roomID)
        settings_to_delete = session.scalar(query)

        if settings_to_delete is None:
            return False

        if room_to_delete is None:
            return False
        

        # Remove child tables first
        session.delete(settings_to_delete)
        session.flush()



        session.delete(room_to_delete)

        return True