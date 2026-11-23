from sqlalchemy import String, Integer, LargeBinary, CheckConstraint, ForeignKey
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class UserSettings(Base):
    __tablename__ = "USER_SETTINGS"

    user_id: Mapped[str] = mapped_column(
        String,
        ForeignKey("USER.user_id"),
        primary_key = True,
        nullable = False
    )

    display_name: Mapped[str] = mapped_column(
        String(20),
        nullable = False
    )

    profile_picture: Mapped[str | None] = mapped_column(
        String,
        nullable = True
    )

    notification_prefs: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("notification_prefs == 0 OR notification_prefs == 1"),
        nullable = False
    )

    audio_volume: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("audio_volume >= 0 AND audio_volume <= 100"),
        nullable = False
    )