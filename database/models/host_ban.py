from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class Host_Ban(Base):
    __tablename__ = "HOST_BAN"

    host_ban_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        nullable = False
    )

    owner_user_id: Mapped[str] = mapped_column(
        String,
        foreign_keys(USER.user_id),
        nullable = False
    )

    peer_user_id: Mapped[str] = mapped_column(
        String,
        foreign_keys(KNOWN_PEER.peer_user_id),
        nullable = False
    )

    banned_at: Mapped[str] mapped_column(
        String,
        nullable = False
    )

    ban_reason: Mapped[str] mapped_column(
        String,
        nullable = False
    )