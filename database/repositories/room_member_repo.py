from sqlalchemy import select, func
from sqlalchemy.orm import Session
from database.models.user import User
from database.models.roles import Roles
from database.models.room_member import RoomMember
from database.models.room import Room
from datetime import datetime, timezone

class RoomMemberRepo:
    # Create -----------------------------------------------------------------------
    def create_roomMembership(self, session: Session, memberKey: bytes, roomID: str, roleName: str, roomDisplayName: str) -> RoomMember:
        if not isinstance(memberKey, bytes) or not memberKey.strip():
            raise ValueError("ERROR: create_roomMembership() FAILED, invalid public key")
        
        if not isinstance(roomID, str) or not roomID.strip():
            raise ValueError("ERROR: create_roomMembership() FAILED, invalid room ID")
        
        if not isinstance(roleName, str) or not roleName.strip():
            raise ValueError("ERROR: create_roomMembership() FAILED, invalid role name")

        if not isinstance(roomDisplayName, str) or not roomDisplayName.strip():
            raise ValueError("ERROR: create_roomMembership() FAILED, invalid role name")

        # Names are unqiue in the roles so we search by name
        query = select(Roles).where(Roles.role_name == roleName)
        selected_role = session.scalar(query)

        if selected_role is None:
            raise ValueError("ERROR: create_roomMembership() FAILED, invalid role name")
        


        query = select(RoomMember).where(RoomMember.room_id == roomID, RoomMember.member_public_key == memberKey)
        check_for_existing_room = session.scalar(query)

        if check_for_existing_room is not None:
            raise ValueError("ERROR: create_roomMembership FAILED, user is already a member of the room")



        
        newRoomMembership = RoomMember(member_public_key = memberKey, room_id = roomID, role_id = selected_role.role_id, has_control = 0,  is_muted = 0, joined_at = datetime.now(timezone.utc).isoformat(), room_display_name = roomDisplayName)

        session.add(newRoomMembership)
        session.flush()

        return newRoomMembership


    # Read -----------------------------------------------------------------------

    def get_allMembers(self, session: Session, roomID: str) -> list[RoomMember]:
        query = select(RoomMember).where(RoomMember.room_id == roomID)
        return session.scalars(query).all()
    
    def get_oneMember(self, session: Session, roomID: str, memberPublicKey: bytes) -> RoomMember | None:
        query = select(RoomMember).where(RoomMember.member_public_key == memberPublicKey, RoomMember.room_id == roomID)
        return session.scalar(query)

    def get_memberCount(self, session: Session, roomID: str) -> int:
        query = select(func.count()).select_from(RoomMember).where(RoomMember.room_id == roomID)
        return session.scalar(query)
    

    def get_controlHolder(self, session: Session, roomID: str) -> RoomMember:
        query = select(RoomMember).where(RoomMember.room_id == roomID, RoomMember.has_control == 1)
        return session.scalar(query)
        
        

    # Update -----------------------------------------------------------------------

    def update_memberRole(self, session: Session, roomID: str, memberPublicKey: bytes, newRole: str) -> RoomMember | None:
        selected_member = self.get_oneMember(session, roomID, memberPublicKey)

        query_for_role = select(Roles).where(Roles.role_name == newRole)
        selected_role = session.scalar(query_for_role)

        if selected_role is None:
            raise ValueError("ERROR: update_memberRole FAILED")


        if selected_member is None:
            return False
        
        if not isinstance(newRole, str) or not newRole:
            raise ValueError("ERROR:update_memberRole() FAILED, invalid role name")
        
        selected_member.role_id = selected_role.role_id

        return selected_member
    

    def update_isMuted(self, session: Session, roomID: str, memberPublicKey: str, isMuted: bool) -> RoomMember | None:
        selected_member = self.get_oneMember(session, roomID, memberPublicKey)

        if selected_member is None:
            return False
        
        if type(isMuted) is not bool:
            raise ValueError("ERROR:update_memberRole() FAILED, invalid role name")
        
        selected_member.is_muted = int(isMuted)

        return selected_member
    


    def set_controlHolder(self, session: Session, roomID: str, memberPublicKey: bytes) -> RoomMember | None:
        session.flush()

        query = select(RoomMember).where(RoomMember.room_id == roomID, RoomMember.member_public_key == memberPublicKey)
        selected_member = session.scalar(query)

        if selected_member is None:
            return None
        
        all_members = self.get_allMembers(session, roomID)

        for member in all_members:
            member.has_control = 0
        
        session.flush()

        selected_member.has_control = 1

        return selected_member

    # Delete -----------------------------------------------------------------------

    def kick_roomMember(self, session: Session, roomID: str, memberPublicKey: bytes) -> bool:

        selected_member = self.get_oneMember(session, roomID, memberPublicKey)

        if selected_member is None:
            return False
        


        session.delete(selected_member)
        session.flush()
        return True
