from sqlalchemy import String, LargeBinary, CheckConstraint, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class KnownPeer(Base):
    __tablename__ = "KNOWN_PEER"

    owner_user_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("USER.user_id"),
        primary_key = True,
        nullable = False
    )

    peer_public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        primary_key = True,
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

