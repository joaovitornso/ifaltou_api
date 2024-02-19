from django.urls import path
from . import views

urlpatterns = [
    path('receber-json/', views.receber_json, name='receber_json'),
]