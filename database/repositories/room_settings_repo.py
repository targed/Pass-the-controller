from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.room import Room
from database.models.room_settings import RoomSettings

# max_member_limit, is_open
class RoomSettingsRepo:
    # Create -----------------------------------------------------------------------

    def create_roomSettings(self, session: Session, roomID: str) -> RoomSettings:
        if not isinstance(roomID, str) or not roomID.strip():
            raise ValueError("ERROR: create_roomSettings() FAILED, invalid room id")


        query = select(Room).where(Room.room_id == roomID)
        room_to_check = session.scalar(query)

        if room_to_check is None:
            raise ValueError("ERROR: create_roomSettings() FAILED, room does not exist")

        
        query = select(RoomSettings).where(RoomSettings.room_id == roomID)
        check_for_settings = session.scalar(query)

        if check_for_settings is not None:
            raise ValueError( "ERROR: create_roomSettings() FAILED, settings already exist")

       
        newRoomSettings = RoomSettings(room_id = roomID, max_member_limit = 10, is_open = 1)


        session.add(newRoomSettings)
        return newRoomSettings
    
    # Read -----------------------------------------------------------------------

    def get_maxMemberLimit(self, session: Session, roomID: str) -> int:
        query = select(RoomSettings.max_member_limit).where(RoomSettings.room_id == roomID)
        return session.scalar(query)

    def get_isOpen(self, session: Session, roomID: str) -> int:
        query = select(RoomSettings.is_open).where(RoomSettings.room_id == roomID)
        return session.scalar(query)
    
    # Update -----------------------------------------------------------------------

    def update_maxMemberLimit(self, session: Session, roomID: str, newMaxLimit: int) -> RoomSettings | None:

        if type(newMaxLimit) is not int or newMaxLimit < 1:
            raise ValueError("ERROR: update_maxMemberLimit() FAILED, member limit must be a positive number")

        query = select(RoomSettings).where(RoomSettings.room_id == roomID)
        max_limit = session.scalar(query)

        if max_limit is None:
            return None        


        max_limit.max_member_limit = newMaxLimit
        

        return max_limit
    

    def update_isOpen(self, session: Session, roomID: str) -> bool:
        query = select(RoomSettings).where(RoomSettings.room_id == roomID)
        open_room = session.scalar(query)

        if open_room is None:
            return None
        
    
        open_room.is_open = int(not open_room.is_open) # Toggle
        return open_room

