from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Known_Peer(Base):
    __tablename__ = "KNOWN_PEER"

    owner_user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys("USER.user_id"),
        nullable = False
    )

    peer_user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        nullable = False
    )

    peer_public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        nullable = False
    )

    first_seen: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    last_seen: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

