from sqlalchemy import String, LargeBinary, CheckConstraint, ForeignKey, ForeignKeyConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class HostBan(Base):
    __tablename__ = "HOST_BAN"
    __table_args__ = (ForeignKeyConstraint(["owner_user_id", "peer_public_key"], ["KNOWN_PEER.owner_user_id", "KNOWN_PEER.peer_public_key"]),)
    

    
    owner_user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        nullable = False
    )

    peer_public_key: Mapped[bytes] = mapped_column(
        LargeBinary,
        primary_key = True,
        nullable = False
    )

    banned_at: Mapped[str] = mapped_column(
        String,
        nullable = False
    )

    ban_reason: Mapped[str | None] = mapped_column(
        String,
        nullable = True
    )