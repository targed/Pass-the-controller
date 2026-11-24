from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Permissions(Base):
    __tablename__ = "PERMISSIONS"

    permission_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        nullable = False
    )

    perm_name: Mapped[str] = mapped_column(
        String,
        unique = True,
        nullable = False
    )