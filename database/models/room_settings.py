from sqlalchemy import String, LargeBinary, CheckConstraint, ForeignKey, Integer
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class RoomSettings(Base):
    __tablename__ = "ROOM_SETTINGS"

    room_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("ROOM.room_id"),
        primary_key = True,
        nullable = False
    )

    max_member_limit: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("max_member_limit >= 1"),
        nullable = False
    )

    is_open: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("is_open IN (0,1)"),
        nullable = False
    )