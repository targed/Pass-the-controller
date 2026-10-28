from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Roles(Base):
    __tablename__ = "ROLES"

    role_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        unique = True,
        nullable = False
    )

    role_name: Mapped[str] = mapped_column(
        String,
        unique = True,
        nullable = False
    )

