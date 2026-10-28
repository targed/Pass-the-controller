from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Room_Member(Base):
    __tablename__ = "ROOM_MEMBER"

    user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys(USER.user_id),
        nullable = False
    )

    room_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys(ROOM.room_id),
        nullable = False
    )

    role_id: Mapped[str] = mapped_column(
        String,
        foreign_keys(ROLES.role_id),
        nullable = False
    )

    has_control: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("has_control == 0 OR has_control == 1")
        nullable = False
    )

    is_muted: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("is_muted == 0 OR is_muted == 1")
        nullable = False
    )

    joined_at: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    