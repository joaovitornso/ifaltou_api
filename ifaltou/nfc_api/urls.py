from django.urls import path
from . import views

urlpatterns = [
    path('receber-json/', views.receber_json, name='receber_json'),
    path('send-buffer/', views.get_buffer, name='get_buffer'),
    path('nfc-reading/', views.NFCAPIListView.as_view(), name='nfc-api-list'),
]