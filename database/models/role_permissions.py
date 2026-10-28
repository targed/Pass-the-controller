from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Role_Permissions(Base):
    __tablename__ = "ROLES"

    role_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys(ROLES.role_id)
        nullable = False
    )

    permissions_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys(PERMISSIONS.permissions_id)
        nullable = False
    )