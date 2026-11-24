from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.roles import Roles
from uuid import uuid4


class RolesRepo:
    # Create -----------------------------------------------------------------------
    def create_role(self, session: Session, roleName: str) -> Roles:

        if not isinstance(roleName, str) or not roleName.strip():
            raise ValueError("ERROR: create_role() FAILED, invalid role name")


        query = select(Roles.role_name).where(Roles.role_name == roleName)
        find_role_dup = session.scalar(query)

        # Two roles can't have the same name. If that does not occur, find_role_dup should be None
        if find_role_dup is not None:
            raise ValueError("ERROR: create_role() FAILED, role name already exists")

        newRole = Roles(role_id = str(uuid4()),  role_name = roleName)
        session.add(newRole)

        return newRole
    
    # Read -----------------------------------------------------------------------

    def get_role(self, session: Session, nameOfRole: str) -> Roles  | None:

        query = select(Roles).where(Roles.role_name == nameOfRole)
        role_to_get = session.scalar(query)

        if role_to_get is None:
            return None
      
        
        return role_to_get
    
    def get_all_roles(self, session: Session) -> list[Roles] | None:
        query = select(Roles)
        
        return list(session.scalars(query).all())



    # Update -----------------------------------------------------------------------

    def update_role(self, session: Session, currentName: str, newName: str) -> Roles | None:
        role_to_update = self.get_role(session, currentName)

        if role_to_update is None:
            return None
        

        if not isinstance(newName, str) or not newName.strip():
            raise ValueError("ERROR: update_role() FAILED, role name already exists")
        

        new_role = self.get_role(session, newName)

        if new_role is not None:
            if new_role.role_id != role_to_update.role_id:
                raise ValueError("ERROR: update_role() FAILED, role already exists")


        role_to_update.role_name = newName
        session.commit()

        return role_to_update
    


    # Delete -----------------------------------------------------------------------

    def delete_role(self, session: Session, nameOfRole: str) -> bool:

        role_to_delete = self.get_role(session, nameOfRole)

        if role_to_delete is None:
            return False
        

       
        session.delete(role_to_delete)
        

        return True