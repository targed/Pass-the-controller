from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.permissions import Permissions
from uuid import uuid4

class PermissionsRepo:
   # Create -----------------------------------------------------------------------
    def create_permission(self, session: Session, permissionName: str) -> Permissions:

        if not isinstance(permissionName, str) or not permissionName.strip():
            raise ValueError("ERROR: create_permission() FAILED, invalid permission name")


        query = select(Permissions.perm_name).where(Permissions.perm_name == permissionName)
        find_perm_dup = session.scalar(query)

        # Two perms can't have the same name. If that does not occur, find_perm_dup should be None
        if find_perm_dup is not None:
            raise ValueError("ERROR: create_permission() FAILED, role name already exists")

        newPerm = Permissions(permission_id = str(uuid4()),  perm_name = permissionName)
        session.add(newPerm)

        return newPerm
    
    # Read -----------------------------------------------------------------------

    def get_perm(self, session: Session, nameOfPerm: str) -> Permissions  | None:

        query = select(Permissions).where(Permissions.perm_name == nameOfPerm)
        perm_to_get = session.scalar(query)

        if perm_to_get is None:
            return None
      
        
        return perm_to_get
    
    def get_all_perms(self, session: Session) -> list[Permissions]:
        query = select(Permissions)
        
        return list(session.scalars(query).all())



    # Update -----------------------------------------------------------------------

    def update_perm(self, session: Session, currentName: str, newName: str) -> Permissions | None:
        perm_to_update = self.get_perm(session, currentName)

        if perm_to_update is None:
            return None
        

        if not isinstance(newName, str) or not newName.strip():
            raise ValueError("ERROR: update_perm() FAILED, permission name already exists")
        

        new_perm = self.get_perm(session, newName)

        # new_perm is temporary perm to test if the perm_id from the original is carrying over to the new one
        if new_perm is not None:
            if new_perm.permission_id != perm_to_update.permission_id:
                raise ValueError("ERROR: update_perm() FAILED, permission already exists")


        perm_to_update.perm_name = newName

        return perm_to_update
    


    # Delete -----------------------------------------------------------------------

    def delete_perm(self, session: Session, nameOfPerm: str) -> bool:

        perm_to_delete = self.get_perm(session, nameOfPerm)

        if perm_to_delete is None:
            return False
        

       
        session.delete(perm_to_delete)
        

        return True


