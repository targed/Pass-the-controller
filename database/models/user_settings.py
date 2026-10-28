from sqlalchemy import String, LargeBinary, CheckConstraint
from sqlalchemy.orm import Mapped, mapped_column
from database.base import Base

# User Settings table
class User_Settings(Base):
    __tablename__ = "USER_SETTINGS"

    user_id: Mapped[str] = mapped_column(
        String,
        primary_key = True,
        foreign_keys("USER.user_id"),
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

    notification_perfs: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("notification_perfs == 0 OR notification_perfs == 1"),
        nullable = False
    )

    audio_volume: Mapped[int] = mapped_column(
        Integer,
        CheckConstraint("audio_volume >= 0 AND audio_volume <= 100"),
        nullable = False
    )