from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.role_permissions import RolePermission
from database.models.permissions import Permissions
from database.repositories.roles_repo import RolesRepo
from database.repositories.permissions_repo import PermissionsRepo


# Needs a way to tell if a role and perm combo already exists
# Its ok to have more than one entry of the same role or perm, just not more than one of the same combo
class RolePermissionRepo:
    # Create -----------------------------------------------------------------------
    def create_rolePerm(self, session: Session, nameOfRole: str, nameOfPerm: str) -> RolePermission:
        if not isinstance(nameOfRole, str) or not nameOfRole.strip():
            raise ValueError("ERROR: create_rolePerm() FAILED, invalid role name")
        
        if not isinstance(nameOfPerm, str) or not nameOfPerm.strip():
            raise ValueError("ERROR: create_rolePerm() FAILED, invalid permission name")

        roles_repo = RolesRepo()
        perms_repo = PermissionsRepo()
        
        selected_role = roles_repo.get_role(session, nameOfRole)
        selected_perm = perms_repo.get_perm(session, nameOfPerm)

        if selected_role is None:
            raise ValueError("ERROR: create_rolePerm() FAILED, role does not exist")
        
        if selected_perm is None:
            raise ValueError("ERROR: create_rolePerm() FAILED, permission does not exist")


        query = select(RolePermission).where(RolePermission.role_id == selected_role.role_id, RolePermission.permission_id == selected_perm.permission_id)
        selected_role_perm = session.scalar(query)

        if selected_role_perm is not None:
            raise ValueError("ERROR: create_rolePerm() FAILED, role permission is already assigned")
        
        newRolePerm = RolePermission(role_id = selected_role.role_id, permission_id = selected_perm.permission_id)

        session.add(newRolePerm)
        return newRolePerm
    

    # Read -----------------------------------------------------------------------
    def get_rolePerm(self, session: Session, nameOfRole: str) -> dict[str, str | list[str]]:
        if not isinstance(nameOfRole, str) or not nameOfRole.strip():
            raise ValueError("ERROR: get_rolePerm() FAILED, invalid role name")
        
        roles_repo = RolesRepo()

        selected_role = roles_repo.get_role(session, nameOfRole)

        if selected_role is None:
            return {"role": nameOfRole, "permissions": []}



        # Since role names must be unique, we can query by names intead of id
        query = select(Permissions.perm_name).join(RolePermission, Permissions.permission_id == RolePermission.permission_id).where(RolePermission.role_id == selected_role.role_id).order_by(Permissions.perm_name)

        all_perm_names = list(session.scalars(query).all())

        return {"role": selected_role.role_name, "permissions": all_perm_names}
    

    
    # Delete -----------------------------------------------------------------------

    # We need to know both the role and perm so that we only remove that specific combo and not every entry of a certain role or perm
    def delete_rolePerm(self, session: Session, nameOfRole: str, nameOfPerm) -> bool:
        if not isinstance(nameOfRole, str) or not nameOfRole.strip():
            raise ValueError("ERROR: get_rolePerm() FAILED, invalid role name")
         
        if not isinstance(nameOfPerm, str) or not nameOfPerm.strip():
            raise ValueError("ERROR: create_rolePerm() FAILED, invalid permission name")
        

        # Verify the role and perm combo exists
        roles_repo = RolesRepo()
        perms_repo = PermissionsRepo()

        selected_role = roles_repo.get_role(session, nameOfRole)
        selected_perm =  perms_repo.get_perm(session, nameOfPerm)

        if selected_role is None or selected_perm is None:
            return False

        query = select(RolePermission).where(RolePermission.role_id == selected_role.role_id, RolePermission.permission_id == selected_perm.permission_id)
        selected_role_perm = session.scalar(query)


        if selected_role_perm is None:
            return False

        session.delete(selected_role_perm)


        return True



    

