from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Room(Base):
    __tablename__ = "ROOM"

    room_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        nullable = False
    )

    owner_public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        nullable = False
    )

    room_name: Mapped[str] = mapped_column(
        String,
        nullable = False
    )