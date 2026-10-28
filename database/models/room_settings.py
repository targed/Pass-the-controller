from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Room_Settings(Base):
    __tablename__ = "ROOM_SETTINGS"

    room_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys(ROOM.room_id)
        nullable = False
    )

    max_member_limit: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("max_member_limit >= 0")
        nullable = False
    )

    is_open: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("is_open == 0 OR is_open == 1")
        nullable = False
    )