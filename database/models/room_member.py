from sqlalchemy import String, LargeBinary, CheckConstraint, ForeignKey, Integer
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base



class RoomMember(Base):
    __tablename__ = "ROOM_MEMBER"

    member_public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        primary_key = True,
        nullable = False
    )

    room_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("ROOM.room_id"),
        primary_key = True,
        nullable = False
    )

    role_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("ROLES.role_id"),
        nullable = False
    )

    has_control: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("has_control IN (0,1)"),
        nullable = False
    )

    is_muted: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("is_muted IN (0,1)"),
        nullable = False
    )

    joined_at: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    room_display_name: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    