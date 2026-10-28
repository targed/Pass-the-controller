from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Room(Base):
    __tablename__ = "ROOM"

    room_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        unique = True,
        nullable = False
    )