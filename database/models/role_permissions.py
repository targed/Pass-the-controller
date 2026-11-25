from sqlalchemy import String, LargeBinary, CheckConstraint, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class RolePermission(Base):
    __tablename__ = "ROLES_PERMISSIONS"
   


    role_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("ROLES.role_id"),
        primary_key = True,
        nullable = False
    )

    permission_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("PERMISSIONS.permission_id"),
        primary_key = True,
        nullable = False
    )